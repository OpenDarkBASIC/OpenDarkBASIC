#include "odb-compiler/commands/plugin_info.h"
#include "odb-sdk/fs.h"
#include "odb-sdk/log.h"

//VEC_DEFINE_API(plugin_list, struct plugin_info, 16)
static int plugin_list_realloc(struct plugin_list* v, int16_t elems)         
    {                      
        void* new_mem = mem_realloc(                                           
            v->mem,                                                            
            sizeof(*v->mem) + sizeof(v->mem->data[0]) * (elems - 1));                            
        if (new_mem == NULL)                                                   
        {                                                                      
            log_sdk_err(                                                       
                "Failed to allocate memory in " "pugin_list"                        
                "_reserve(%" PRId16 ")n",                                 
                elems);                                                        
            return -1;                                                         
        }                                                                      
        v->mem = new_mem;                                                  
        return 0;                                                              
    }                                                                          
                                                                               
    int plugin_list_reserve(struct plugin_list* v, int16_t elems)                
    {                                                                          
        if (plugin_list_realloc(v, elems) != 0)                                   
            return -1;                                                         
        v->mem->count = 0;                                                     
        v->mem->capacity = elems;                                              
        return 0;                                                              
    }                                                                          
    void plugin_list_compact(struct plugin_list* v)                                    
    {                                                                          
        if (v->mem == NULL)                                                    
            return;                                                            
                                                                               
        if (v->mem->count == 0)                                                
        {                                                                      
            mem_free(v->mem);                                                  
            v->mem = NULL;                                                     
        }                                                                      
        else                                                                   
        {                                                                      
            void* new_mem = mem_realloc(                                       
                v->mem,                                                        
                sizeof(v->mem->count) + sizeof(v->mem->capacity)               
                    + sizeof(v->mem->data[0]) * v->mem->count);                
            v->mem = new_mem;                                              
            v->mem->capacity = v->mem->count;                                  
        }                                                                      
    }                                                                          
    struct plugin_info* plugin_list_emplace(struct plugin_list* v)                                      
    {                                                                          
        if (v->mem == NULL)                                                    
            if (plugin_list_reserve(v, ODBSDK_VEC_MIN_CAPACITY) != 0)             
                return NULL;                                                   
                                                                               
        if (v->mem->count == v->mem->capacity)                                 
        {                                                                      
            if (plugin_list_realloc(v, v->mem->capacity * ODBSDK_VEC_EXPAND_FACTOR) 
                != 0)                                                          
                return NULL;                                                   
            v->mem->capacity *= ODBSDK_VEC_EXPAND_FACTOR;                      
        }                                                                      
                                                                               
        return &v->mem->data[(v->mem->count)++];                               
    }                                                                          
    struct plugin_info* plugin_list_insert_emplace(struct plugin_list* v, int16_t i)              
    {                                                                          
        if (plugin_list_emplace(v) == NULL)                                       
            return NULL;                                                       
                                                                               
        memmove(                                                               
            v->mem->data + i + 1,                                              
            v->mem->data + i,                                                  
            (v->mem->count - 1) * sizeof(v->mem->data[0]));                    
        return &v->mem->data[i];                                               
    }                                                                          
    void plugin_list_erase(struct plugin_list* v, int16_t i)                     
    {                                                                          
        memmove(                                                               
            v->mem->data + i,                                                  
            v->mem->data + i + 1,                                              
            (v->mem->count - i) * sizeof(v->mem->data[0]));                    
        --v->mem->count;                                                       
    }

struct on_plugin_entry_ctx
{
    struct plugin_list* plugins;
    struct ospath_view dir;
};

static int
on_plugin_entry(const char* cname, void* user)
{
    struct on_plugin_entry_ctx* ctx = user;
    struct ospath_view name = cstr_ospath_view(cname);

    if (ospath_ends_with_i_cstr(name, ".dll")
        || ospath_ends_with_i_cstr(name, ".so"))
    {
        struct plugin_info* plugin = plugin_list_emplace(ctx->plugins);
        if (plugin == NULL)
            return -1;
        plugin_info_init(plugin);

        if (ospath_set(&plugin->filepath, ctx->dir) != 0)
            return -1;
        if (ospath_join(&plugin->filepath, name) != 0)
            return -1;

        if (utf8_set(&plugin->name, &plugin->name_range, name.str, name.range) != 0)
            return -1;
        /* Remove extension */
        while (plugin->name_range.len-- && plugin->name.data[plugin->name_range.off + plugin->name_range.len] != '.') {}
    }
    
    return 0;
}

int
plugin_list_populate(
    struct plugin_list* plugins,
    struct ospath_view  sdk_root,
    const struct ospath_list* extra_plugin_dirs)
{
    struct ospath path = ospath();
    struct on_plugin_entry_ctx ctx = { plugins, {0} };

    if (!fs_dir_exists(sdk_root))
    {
        log_sdk_err(
            "SDK root directory {quote:%s} does not existn",
            ospath_view_cstr(sdk_root));
        return -1;
    }

    if (ospath_set(&path, sdk_root) != 0
        || ospath_join_cstr(&path, "plugins") != 0)
    {
        ospath_deinit(path);
        return -1;
    }
    ctx.dir = ospath_view(path);
    fs_list(ospath_view(path), on_plugin_entry, &ctx);

    ospath_deinit(path);

    return 0;
}
