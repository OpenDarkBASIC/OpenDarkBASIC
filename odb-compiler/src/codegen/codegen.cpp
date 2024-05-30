#include "llvm/IR/Intrinsics.h"
#include "odb-compiler/sdk/sdk.h"
extern "C" {
#include "odb-compiler/ast/ast.h"
#include "odb-compiler/codegen/codegen.h"
#include "odb-compiler/parser/db_parser.y.h"
#include "odb-compiler/sdk/cmd_list.h"
#include "odb-sdk/log.h"
}

#include "llvm/ADT/APInt.h"
#include "llvm/ADT/StringMap.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/GlobalVariable.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/InlineAsm.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Verifier.h"
#include "llvm/MC/TargetRegistry.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Target/TargetOptions.h"
#include "llvm/TargetParser/Host.h"

static std::string
to_string(const llvm::Type* ty)
{
    std::string              str;
    llvm::raw_string_ostream rso(str);
    ty->print(rso);
    return rso.str();
}

static int
create_global_string_table(
    llvm::StringMap<llvm::GlobalVariable*>* string_table,
    const struct ast*                       ast,
    struct db_source                        source,
    llvm::Module*                           mod)
{
    for (ast_id n = 0; n != ast->node_count; ++n)
    {
        if (ast->nodes[n].info.node_type != AST_STRING_LITERAL)
            continue;

        struct utf8_span str = ast->nodes[n].string_literal.str;
        llvm::StringRef  str_ref(source.text.data + str.off, str.len);

        auto result = string_table->try_emplace(str_ref, nullptr);
        if (result.second == false)
            continue; // String already exists

        llvm::Constant* S = llvm::ConstantDataArray::getString(
            mod->getContext(),
            str_ref,
            /* Add NULL */ true);
        result.first->setValue(new llvm::GlobalVariable(
            *mod,
            // llvm::PointerType::get(llvm::Type::getInt8Ty(mod->getContext()),
            // 0),
            S->getType(),
            /*isConstant*/ true,
            llvm::GlobalValue::PrivateLinkage,
            S,
            llvm::Twine(".str") + llvm::Twine(string_table->size() - 1)));
        result.first->getValue()->setAlignment(llvm::Align::Constant<1>());
    }

    return 0;
}

static llvm::Type*
dbpro_cmd_param_type_to_llvm(enum type type, llvm::Module* mod)
{
    switch (type)
    {
        case TYPE_INVALID: break;

        case TYPE_VOID: return llvm::Type::getVoidTy(mod->getContext());
        case TYPE_LONG: return llvm::Type::getInt64Ty(mod->getContext());

        case TYPE_DWORD:
        case TYPE_INTEGER: return llvm::Type::getInt32Ty(mod->getContext());

        case TYPE_WORD: return llvm::Type::getInt16Ty(mod->getContext());

        case TYPE_BYTE:
        case TYPE_BOOLEAN: return llvm::Type::getInt8Ty(mod->getContext());

        case TYPE_FLOAT: return llvm::Type::getFloatTy(mod->getContext());
        case TYPE_DOUBLE: return llvm::Type::getDoubleTy(mod->getContext());

        case TYPE_STRING:
        case TYPE_ARRAY:
            return llvm::PointerType::get(
                llvm::Type::getInt8Ty(mod->getContext()), 0);

        case TYPE_LABEL:
        case TYPE_DABEL: break;

        case TYPE_ANY:
        case TYPE_USER_DEFINED_VAR_PTR:
            return llvm::PointerType::get(
                llvm::Type::getVoidTy(mod->getContext()), 0);
    }

    log_err(
        "[gen] ",
        "Don't know how to convert DBPro type {quote:%c} to LLVM\n",
        type);
    return nullptr;
}

static llvm::FunctionType*
get_command_function_signature(
    const struct ast*      ast,
    ast_id                 cmd,
    const struct cmd_list* cmds,
    llvm::Module*          mod)
{
    ODBSDK_DEBUG_ASSERT(ast->nodes[cmd].info.node_type == AST_COMMAND);
    cmd_id cmd_id = ast->nodes[cmd].cmd.id;

    /* Get command arguments from command list and convert each one to LLVM */
    struct param_types_list* odb_param_types
        = vec_get(cmds->param_types, cmd_id);
    llvm::SmallVector<llvm::Type*, 8> llvm_param_types;
    struct cmd_param*                 odb_param;
    vec_for_each(*odb_param_types, odb_param)
    {
        llvm_param_types.push_back(
            dbpro_cmd_param_type_to_llvm(odb_param->type, mod));
    }

    /* Convert return type from command list as well, and create LLVM FT */
    enum type odb_return_type = *vec_get(cmds->return_types, cmd_id);
    return llvm::FunctionType::get(
        dbpro_cmd_param_type_to_llvm(odb_return_type, mod),
        llvm_param_types,
        /* isVarArg */ false);
}

static int
create_global_plugin_symbol_table(
    llvm::StringMap<llvm::Function*>* plugin_symbol_table,
    const struct ast*                 ast,
    struct db_source                  source,
    const struct cmd_list*            cmds,
    llvm::Module*                     mod)
{
    for (ast_id n = 0; n != ast->node_count; ++n)
    {
        if (ast->nodes[n].info.node_type != AST_COMMAND)
            continue;

        cmd_id           cmd_id = ast->nodes[n].cmd.id;
        struct utf8_view c_sym = utf8_list_view(&cmds->c_symbols, cmd_id);
        llvm::StringRef  c_sym_ref(c_sym.data + c_sym.off, c_sym.len);

        auto result = plugin_symbol_table->try_emplace(c_sym_ref, nullptr);
        if (result.second == false) // Command already in table
            continue;

        result.first->setValue(llvm::Function::Create(
            get_command_function_signature(ast, n, cmds, mod),
            llvm::Function::ExternalLinkage,
            c_sym_ref,
            mod));
    }

    return 0;
}

ODBSDK_PRINTF_FORMAT(4, 5)
static void
log_semantic_err(
    const char*      filename,
    struct db_source source,
    struct utf8_span location,
    const char*      fmt,
    ...)
{
    va_list ap;
    va_start(ap, fmt);
    log_vflc(
        "{e:semantic error:} ", filename, source.text.data, location, fmt, ap);
    va_end(ap);
    log_excerpt(filename, source.text.data, location, "");
}

static llvm::Value*
gen_expr(
    const struct ast*                             ast,
    ast_id                                        expr,
    const struct cmd_list*                        cmds,
    const char*                                   source_filename,
    struct db_source                              source,
    const llvm::StringMap<llvm::GlobalVariable*>* string_table,
    const llvm::StringMap<llvm::Function*>*       plugin_symbol_table,
    llvm::Module*                                 mod,
    llvm::BasicBlock*                             BB);

static llvm::Value*
gen_cmd_call(
    const struct ast*                             ast,
    ast_id                                        cmd,
    const struct cmd_list*                        cmds,
    const char*                                   source_filename,
    struct db_source                              source,
    const llvm::StringMap<llvm::GlobalVariable*>* string_table,
    const llvm::StringMap<llvm::Function*>*       plugin_symbol_table,
    llvm::Module*                                 mod,
    llvm::BasicBlock*                             BB)
{
    ast_id arglist;
    size_t a;
    cmd_id cmd_id = ast->nodes[cmd].cmd.id;

    // Function table for commands should be generated at this
    // point. Look up the command's symbol in the command list and
    // get the associated llvm::Function
    struct utf8_view cmd_sym = utf8_list_view(&cmds->c_symbols, cmd_id);
    llvm::StringRef  cmd_sym_ref(cmd_sym.data + cmd_sym.off, cmd_sym.len);
    llvm::Function*  F = plugin_symbol_table->find(cmd_sym_ref)->getValue();

    // Match up each function argument with its corresponding
    // parameter.
    // Command overload resolution is done in a previous step, so
    // it's OK to assume that both lists have the same length and matching
    // types.
    llvm::SmallVector<llvm::Value*, 8> param_values;
    for (a = 0, arglist = ast->nodes[cmd].cmd.arglist; a != F->arg_size();
         ++a, arglist = ast->nodes[arglist].arglist.next)
    {
        param_values.push_back(gen_expr(
            ast,
            ast->nodes[arglist].arglist.expr,
            cmds,
            source_filename,
            source,
            string_table,
            plugin_symbol_table,
            mod,
            BB));
    }

    llvm::IRBuilder<> b(mod->getContext());
    b.SetInsertPoint(BB);
    return b.CreateCall(F, param_values);
}

static llvm::Value*
gen_expr(
    const struct ast*                             ast,
    ast_id                                        expr,
    const struct cmd_list*                        cmds,
    const char*                                   source_filename,
    struct db_source                              source,
    const llvm::StringMap<llvm::GlobalVariable*>* string_table,
    const llvm::StringMap<llvm::Function*>*       plugin_symbol_table,
    llvm::Module*                                 mod,
    llvm::BasicBlock*                             BB)
{
    llvm::IRBuilder<> b(mod->getContext());
    b.SetInsertPoint(BB);

    switch (ast->nodes[expr].info.node_type)
    {
        case AST_BLOCK:
        case AST_ARGLIST:
        case AST_CONST_DECL: break;

        case AST_COMMAND:
            return gen_cmd_call(
                ast,
                expr,
                cmds,
                source_filename,
                source,
                string_table,
                plugin_symbol_table,
                mod,
                BB);

        case AST_ASSIGNMENT:
        case AST_IDENTIFIER: break;

        case AST_BINOP: {
            ast_id       lhs_node = ast->nodes[expr].binop.left;
            ast_id       rhs_node = ast->nodes[expr].binop.right;
            enum type    lhs_type = ast->nodes[lhs_node].info.type_info;
            enum type    rhs_type = ast->nodes[rhs_node].info.type_info;
            enum type    result_type = ast->nodes[expr].binop.info.type_info;
            llvm::Value* lhs = gen_expr(
                ast,
                lhs_node,
                cmds,
                source_filename,
                source,
                string_table,
                plugin_symbol_table,
                mod,
                BB);
            llvm::Value* rhs = gen_expr(
                ast,
                rhs_node,
                cmds,
                source_filename,
                source,
                string_table,
                plugin_symbol_table,
                mod,
                BB);

            /* Handle string operations seperately from arithmetic, since there
             * are only a handful of ops that are valid */
            if (result_type == TYPE_STRING)
            {
                // TODO
                return nullptr;
            }

            enum TypeFamily
            {
                INT,
                UINT,
                FLOAT
            } type_family
                = INT;
            switch (result_type)
            {
                case TYPE_INVALID:
                case TYPE_VOID:
                case TYPE_STRING:
                case TYPE_ARRAY:
                case TYPE_LABEL:
                case TYPE_DABEL:
                case TYPE_ANY:
                case TYPE_USER_DEFINED_VAR_PTR:
                    ODBSDK_DEBUG_ASSERT(false);
                    return nullptr;

                case TYPE_BOOLEAN:
                case TYPE_INTEGER:
                case TYPE_LONG: type_family = INT; break;

                case TYPE_DWORD:
                case TYPE_WORD:
                case TYPE_BYTE: type_family = UINT; break;

                case TYPE_FLOAT:
                case TYPE_DOUBLE: type_family = FLOAT; break;
            }

            switch (ast->nodes[expr].binop.op)
            {
                case BINOP_ADD:
                    switch (type_family)
                    {
                        case INT: return b.CreateNSWAdd(lhs, rhs);
                        case UINT: return b.CreateAdd(lhs, rhs);
                        case FLOAT: return b.CreateFAdd(lhs, rhs);
                    }
                    break;
                case BINOP_SUB:
                    switch (type_family)
                    {
                        case INT: return b.CreateNSWSub(lhs, rhs);
                        case UINT: return b.CreateSub(lhs, rhs);
                        case FLOAT: return b.CreateFSub(lhs, rhs);
                    }
                    break;
                case BINOP_MUL:
                    switch (type_family)
                    {
                        case INT: return b.CreateNSWMul(lhs, rhs);
                        case UINT: return b.CreateMul(lhs, rhs);
                        case FLOAT: return b.CreateFMul(lhs, rhs);
                    }
                    break;
                case BINOP_DIV:
                    switch (type_family)
                    {
                        case INT: return b.CreateSDiv(lhs, rhs);
                        case UINT: return b.CreateUDiv(lhs, rhs);
                        case FLOAT: return b.CreateFDiv(lhs, rhs);
                    }
                    break;
                case BINOP_MOD:
                    switch (type_family)
                    {
                        case INT: return b.CreateSRem(lhs, rhs);
                        case UINT: return b.CreateURem(lhs, rhs);
                        case FLOAT: return b.CreateFRem(lhs, rhs);
                    }
                    break;
                case BINOP_POW:
                    switch (lhs_type)
                    {
                        case TYPE_FLOAT: {
                            switch (rhs_type)
                            {
                                case TYPE_INVALID:
                                case TYPE_VOID:
                                case TYPE_STRING:
                                case TYPE_ARRAY:
                                case TYPE_LABEL:
                                case TYPE_DABEL:
                                case TYPE_ANY:
                                case TYPE_USER_DEFINED_VAR_PTR: break;

                                case TYPE_LONG:
                                case TYPE_DWORD:
                                case TYPE_INTEGER:
                                case TYPE_WORD:
                                case TYPE_BYTE:
                                case TYPE_BOOLEAN: {
                                    llvm::Function* FPowi
                                        = llvm::Intrinsic::getDeclaration(
                                            mod,
                                            llvm::Intrinsic::powi,
                                            llvm::Type::getFloatTy(
                                                mod->getContext()));
                                    return b.CreateCall(FPowi, {lhs, rhs});
                                }
                                case TYPE_FLOAT: {
                                    llvm::Function* FPow
                                        = llvm::Intrinsic::getDeclaration(
                                            mod,
                                            llvm::Intrinsic::pow,
                                            llvm::Type::getFloatTy(
                                                mod->getContext()));
                                    return b.CreateCall(FPow, {lhs, rhs});
                                }
                                case TYPE_DOUBLE: {
                                    llvm::Function* FPow
                                        = llvm::Intrinsic::getDeclaration(
                                            mod,
                                            llvm::Intrinsic::pow,
                                            llvm::Type::getDoubleTy(
                                                mod->getContext()));
                                    return b.CreateCall(FPow, {lhs, rhs});
                                }
                            }
                        }
                        case TYPE_DOUBLE: {
                            switch (rhs_type)
                            {
                                case TYPE_INVALID:
                                case TYPE_VOID:
                                case TYPE_STRING:
                                case TYPE_ARRAY:
                                case TYPE_LABEL:
                                case TYPE_DABEL:
                                case TYPE_ANY:
                                case TYPE_USER_DEFINED_VAR_PTR: break;

                                case TYPE_LONG:
                                case TYPE_DWORD:
                                case TYPE_INTEGER:
                                case TYPE_WORD:
                                case TYPE_BYTE:
                                case TYPE_BOOLEAN: {
                                    llvm::Function* FPowi
                                        = llvm::Intrinsic::getDeclaration(
                                            mod,
                                            llvm::Intrinsic::powi,
                                            llvm::Type::getFloatTy(
                                                mod->getContext()));
                                    return b.CreateCall(FPowi, {lhs, rhs});
                                }
                                case TYPE_FLOAT: {
                                    llvm::Function* FPow
                                        = llvm::Intrinsic::getDeclaration(
                                            mod,
                                            llvm::Intrinsic::pow,
                                            llvm::Type::getFloatTy(
                                                mod->getContext()));
                                    return b.CreateCall(FPow, {lhs, rhs});
                                }
                                case TYPE_DOUBLE: {
                                    llvm::Function* FPow
                                        = llvm::Intrinsic::getDeclaration(
                                            mod,
                                            llvm::Intrinsic::pow,
                                            llvm::Type::getDoubleTy(
                                                mod->getContext()));
                                    return b.CreateCall(FPow, {lhs, rhs});
                                }
                            }
                        }
                    }
                    break;
                case BINOP_SHIFT_LEFT:
                case BINOP_SHIFT_RIGHT:
                case BINOP_BITWISE_OR:
                case BINOP_BITWISE_AND:
                case BINOP_BITWISE_XOR:
                case BINOP_BITWISE_NOT:
                case BINOP_LESS_THAN:
                case BINOP_LESS_EQUAL:
                case BINOP_GREATER_THAN:
                case BINOP_GREATER_EQUAL:
                case BINOP_EQUAL:
                case BINOP_NOT_EQUAL:
                case BINOP_LOGICAL_OR:
                case BINOP_LOGICAL_AND:
                case BINOP_LOGICAL_XOR: break;
            }
        }
        break;
        case AST_UNOP: break;

        case AST_BOOLEAN_LITERAL:
            return llvm::ConstantInt::get(
                llvm::Type::getInt8Ty(mod->getContext()),
                ast->nodes[expr].boolean_literal.is_true,
                /* isSigned */ true);

        case AST_BYTE_LITERAL:
            return llvm::ConstantInt::get(
                llvm::Type::getInt8Ty(mod->getContext()),
                ast->nodes[expr].byte_literal.value,
                /* isSigned */ false);
        case AST_WORD_LITERAL:
            return llvm::ConstantInt::get(
                llvm::Type::getInt16Ty(mod->getContext()),
                ast->nodes[expr].word_literal.value,
                /* isSigned */ false);
        case AST_DWORD_LITERAL:
            return llvm::ConstantInt::get(
                llvm::Type::getInt32Ty(mod->getContext()),
                ast->nodes[expr].dword_literal.value,
                /* isSigned */ false);
        case AST_INTEGER_LITERAL:
            return llvm::ConstantInt::get(
                llvm::Type::getInt32Ty(mod->getContext()),
                ast->nodes[expr].integer_literal.value,
                /* isSigned */ true);
        case AST_DOUBLE_INTEGER_LITERAL:
            return llvm::ConstantInt::get(
                llvm::Type::getInt64Ty(mod->getContext()),
                ast->nodes[expr].double_integer_literal.value,
                /* isSigned */ true);

        case AST_FLOAT_LITERAL:
            return llvm::ConstantFP::get(
                llvm::Type::getFloatTy(mod->getContext()),
                llvm::APFloat(ast->nodes[expr].float_literal.value));
        case AST_DOUBLE_LITERAL:
            return llvm::ConstantFP::get(
                llvm::Type::getDoubleTy(mod->getContext()),
                llvm::APFloat(ast->nodes[expr].double_literal.value));

        case AST_STRING_LITERAL: {
            struct utf8_span span = ast->nodes[expr].string_literal.str;
            llvm::StringRef  str_ref(source.text.data + span.off, span.len);
            return string_table->find(str_ref)->getValue();
        }

        case AST_CAST: {
            /* clang-format off */
            using Op = llvm::Instruction::CastOps;
            Op O = Op::CastOpsBegin;
            static const Op llvm_cast_ops[16][16] = {
     /*          0 R          D          L          W          Y          B           F           O          S H P Q X           E */
              {O,O,           O,         O,         O,         O,         O,          O,          O,         O,O,O,O,O,          O},
     /* 0 */  {O,O,           O,         O,         O,         O,         O,          O,          O,         O,O,O,O,O,          O},
     /* R */  {O,O,Op::SExt,  Op::Trunc, Op::Trunc, Op::Trunc, Op::Trunc, O,          Op::SIToFP, Op::SIToFP,O,O,O,O,Op::BitCast,O},
     /* D */  {O,O,Op::ZExt,  Op::ZExt,  Op::ZExt,  Op::Trunc, Op::Trunc, O,          Op::UIToFP, Op::UIToFP,O,O,O,O,Op::BitCast,O},
     /* L */  {O,O,Op::SExt,  Op::SExt,  Op::SExt,  Op::Trunc, Op::Trunc, O,          Op::SIToFP, Op::SIToFP,O,O,O,O,Op::BitCast,O},
     /* W */  {O,O,Op::ZExt,  Op::ZExt,  Op::ZExt,  Op::ZExt,  Op::Trunc, O,          Op::UIToFP, Op::UIToFP,O,O,O,O,Op::BitCast,O},
     /* Y */  {O,O,Op::ZExt,  Op::ZExt,  Op::ZExt,  Op::ZExt,  Op::ZExt,  O,          Op::UIToFP, Op::UIToFP,O,O,O,O,Op::BitCast,O},
     /* B */  {O,O,Op::SExt,  Op::SExt,  Op::SExt,  Op::SExt,  Op::SExt,  Op::SExt,   Op::SIToFP, Op::SIToFP,O,O,O,O,Op::BitCast,O},
     /* F */  {O,O,Op::FPToSI,Op::FPToUI,Op::FPToSI,Op::FPToUI,Op::FPToUI,O,          Op::FPExt,  Op::FPExt, O,O,O,O,Op::BitCast,O},
     /* O */  {O,O,Op::FPToSI,Op::FPToUI,Op::FPToSI,Op::FPToUI,Op::FPToUI,O,          Op::FPTrunc,Op::FPExt, O,O,O,O,Op::BitCast,O},
     /* S */  {O,O,O,         O,         O,         O,         O,         O,          O,          O,         O,O,O,O,Op::BitCast,O},
     /* H */  {O,O,O,         O,         O,         O,         O,         O,          O,          O,         O,O,O,O,Op::BitCast,O},
     /* P */  {O,O,O,         O,         O,         O,         O,         O,          O,          O,         O,O,O,O,Op::BitCast,O},
     /* Q */  {O,O,O,         O,         O,         O,         O,         O,          O,          O,         O,O,O,O,Op::BitCast,O},
     /* X */  {O,O,O,         O,         O,         O,         O,         O,          O,          O,         O,O,O,O,Op::BitCast,O},
     /* E */  {O,O,O,         O,         O,         O,         O,         O,          O,          O,         O,O,O,O,Op::BitCast,O},
            };
            /* clang-format on */

            llvm::Value* value = gen_expr(
                ast,
                ast->nodes[expr].cast.expr,
                cmds,
                source_filename,
                source,
                string_table,
                plugin_symbol_table,
                mod,
                BB);
            enum type from
                = ast->nodes[ast->nodes[expr].cast.expr].info.type_info;
            enum type to = ast->nodes[expr].cast.info.type_info;
            return b.CreateCast(
                llvm_cast_ops[from][to],
                value,
                dbpro_cmd_param_type_to_llvm(to, mod));
        }
        break;
    }

    log_err(
        "[gen] ",
        "Expression type %d not implemeneted\n",
        ast->nodes[expr].info.node_type);
    return nullptr;
}

static llvm::BasicBlock*
gen_block(
    const struct ast*                             ast,
    ast_id                                        block,
    const struct cmd_list*                        cmds,
    const char*                                   source_filename,
    const struct db_source                        source,
    const llvm::StringMap<llvm::GlobalVariable*>* string_table,
    const llvm::StringMap<llvm::Function*>*       plugin_symbol_table,
    llvm::Module*                                 mod)
{
    ODBSDK_DEBUG_ASSERT(block > -1);
    ODBSDK_DEBUG_ASSERT(ast->nodes[block].info.node_type == AST_BLOCK);

    /* Set up a new BasicBlock which gets filled with all of the DarkBASIC
     * statements from the current node. We name it according to the node's
     * index in the AST. Makes it easier to track down issues later on. */
    llvm::BasicBlock* BB = llvm::BasicBlock::Create(
        mod->getContext(), llvm::Twine("block") + llvm::Twine(block));
    llvm::IRBuilder<> b(mod->getContext());
    b.SetInsertPoint(BB);

    for (; block != -1; block = ast->nodes[block].block.next)
    {
        ast_id stmt = ast->nodes[block].block.stmt;
        ODBSDK_DEBUG_ASSERT(stmt > -1);
        switch (ast->nodes[stmt].info.node_type)
        {
            case AST_COMMAND: {
                gen_cmd_call(
                    ast,
                    stmt,
                    cmds,
                    source_filename,
                    source,
                    string_table,
                    plugin_symbol_table,
                    mod,
                    BB);
                break;
            }

            default:
                log_err(
                    "[gen] ",
                    "Statement type not implemented while translating block\n");
                return nullptr;
        }
    }

    return BB;
}

int
odb_codegen(
    struct ast*                  program,
    const char*                  output_name,
    const char*                  module_name,
    enum sdk_type                sdkType,
    enum odb_codegen_output_type output_type,
    enum odb_codegen_arch        arch,
    enum odb_codegen_platform    platform,
    const struct cmd_list*       cmds,
    const char*                  source_filename,
    struct db_source             source)
{
    llvm::LLVMContext ctx;
    llvm::Module      mod(module_name, ctx);
    llvm::IRBuilder<> b(ctx);

    llvm::StringMap<llvm::GlobalVariable*> string_table;
    create_global_string_table(&string_table, program, source, &mod);

    llvm::StringMap<llvm::Function*> plugin_symbol_table;
    create_global_plugin_symbol_table(
        &plugin_symbol_table, program, source, cmds, &mod);

    // Entry point of the program
    llvm::Function* F = llvm::Function::Create(
        llvm::FunctionType::get(
            llvm::Type::getInt32Ty(ctx),
            {llvm::Type::getInt32Ty(ctx), llvm::PointerType::getUnqual(ctx)},
            false),
        llvm::Function::ExternalLinkage,
        "main",
        &mod);
    // F->setDoesNotReturn();

    // Translate AST
    llvm::BasicBlock* BB = gen_block(
        program,
        0,
        cmds,
        source_filename,
        source,
        &string_table,
        &plugin_symbol_table,
        &mod);
    BB->insertInto(F);
    b.SetInsertPoint(BB);

    // Plugins use odb-sdk. Have to call the global init functions
    if (sdkType == SDK_ODB)
    {
        llvm::Function* FSDKInit = llvm::Function::Create(
            llvm::FunctionType::get(llvm::Type::getInt32Ty(ctx), {}, false),
            llvm::Function::ExternalLinkage,
            "odbsdk_init",
            &mod);
        b.SetInsertPoint(BB->getFirstInsertionPt());
        b.CreateCall(FSDKInit, {});

        llvm::Function* FSDKDeInit = llvm::Function::Create(
            llvm::FunctionType::get(llvm::Type::getVoidTy(ctx), {}, false),
            llvm::Function::ExternalLinkage,
            "odbsdk_deinit",
            &mod);
        b.SetInsertPoint(BB);
        b.CreateCall(FSDKDeInit, {});
    }

    // Exit process call
    // llvm::Function* FExitProcess = llvm::Function::Create(
    //    llvm::FunctionType::get(
    //        llvm::Type::getVoidTy(ctx), {llvm::Type::getInt32Ty(ctx)}, false),
    //    llvm::Function::ExternalLinkage,
    //    platform == ODB_CODEGEN_WINDOWS ? "ExitProcess" : "_exit",
    //    &mod);
    // FExitProcess->setDoesNotReturn();
    // b.CreateCall(FExitProcess, llvm::ConstantInt::get(ctx, llvm::APInt(32,
    // 0)));

    // Finish off block
    // b.CreateRetVoid();
    b.CreateRet(llvm::ConstantInt::get(ctx, llvm::APInt(32, 0)));

    if (platform == ODB_CODEGEN_WINDOWS)
    {
        llvm::Constant* rpath_data = llvm::ConstantDataArray::getString(
            ctx,
            llvm::StringRef("odb-sdk\\plugins\\"),
            /* Add NULL */ true);
        llvm::GlobalVariable* rpath_const = new llvm::GlobalVariable(
            mod,
            rpath_data->getType(),
            /*isConstant*/ true,
            llvm::GlobalValue::PrivateLinkage,
            rpath_data,
            ".rpath");
        rpath_const->setAlignment(llvm::Align::Constant<1>());

        llvm::Function* rpath_func = llvm::Function::Create(
            llvm::FunctionType::get(
                llvm::Type::getInt32Ty(ctx),
                {llvm::PointerType::get(llvm::Type::getInt8Ty(ctx), 0)},
                false),
            llvm::Function::ExternalLinkage,
            "SetDllDirectoryA",
            &mod);

        b.SetInsertPoint(BB->getFirstNonPHI());
        b.CreateCall(rpath_func, rpath_const);
    }

    // Validate the generated code, checking for consistency.
    llvm::verifyFunction(*F);

    mod.print(llvm::outs(), nullptr);

    /* clang-format off */
    static const char* target_triples[3][3] = {
        {"i386-pc-windows-msvc", "x86_64-pc-windows-msvc", ""},
        {"i386-linux-gnu",       "x86_64-linux-gnu", ""},
        {"i386-",                "x86_64-", ""}
    };
    /* clang-format on */

    llvm::InitializeAllTargetInfos();
    llvm::InitializeAllTargets();
    llvm::InitializeAllTargetMCs();
    llvm::InitializeAllAsmParsers();
    llvm::InitializeAllAsmPrinters();

    std::string Error;
    auto        TargetTriple = llvm::sys::getDefaultTargetTriple();
    TargetTriple = target_triples[platform][arch];
    auto Target = llvm::TargetRegistry::lookupTarget(TargetTriple, Error);

    // Print an error and exit if we couldn't find the requested target.
    // This generally occurs if we've forgotten to initialise the
    // TargetRegistry or we have a bogus target triple.
    if (!Target)
    {
        llvm::errs() << Error;
        return -1;
    }

    auto                CPU = "generic";
    auto                Features = "";
    llvm::TargetOptions opt;
    auto                TargetMachine = Target->createTargetMachine(
        TargetTriple, CPU, Features, opt, llvm::Reloc::Static);

    mod.setDataLayout(TargetMachine->createDataLayout());
    mod.setTargetTriple(TargetTriple);

    std::error_code      EC;
    llvm::raw_fd_ostream dest(output_name, EC, llvm::sys::fs::OF_None);
    if (EC)
    {
        llvm::errs() << "Could not open file: " << EC.message();
        return -1;
    }

    llvm::legacy::PassManager pass;
    auto                      FileType = llvm::CodeGenFileType::ObjectFile;
    if (TargetMachine->addPassesToEmitFile(pass, dest, nullptr, FileType))
    {
        llvm::errs() << "TargetMachine can't emit a file of this type";
        return -1;
    }

    pass.run(mod);
    dest.flush();

    return 0;
}
