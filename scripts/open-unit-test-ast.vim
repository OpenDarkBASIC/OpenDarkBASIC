function OpenUnitTestAST(ast_version)
    let l:filename = expand("%:p")
    let l:line = line('.')
    let l:command = "python3 ./scripts/open-unit-test-ast.py " . l:filename . " " . l:line . " " . a:ast_version
    call jobstart(l:command)
endfunction

nnoremap <leader>g1 :call OpenUnitTestAST(1)<CR>
nnoremap <leader>g2 :call OpenUnitTestAST(2)<CR>

