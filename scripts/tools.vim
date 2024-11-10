function OpenUnitTestAST(ast_version)
    let l:filename = expand("%:p")
    let l:line = line('.')
    let l:command = "python3 ./scripts/open-unit-test-ast.py " . l:filename . " " . l:line . " " . a:ast_version
    "silent call jobstart(l:command)
    echo system(l:command)
endfunction

function PatchUnitTest(visual_mode)
    let l:filename = expand("%:p")
    let l:line = line('.')
    let l:command = "python3 ./scripts/patch-unit-test.py " . l:filename . " " . l:line . " " . a:visual_mode

    let l:output = system(l:command)
    if v:shell_error != 0
        echohl ErrorMsg
        echo "Command failed: " . l:command . "\n" . l:output
        echohl NONE
        return
    endif

    let winsaved = winsaveview()
    if a:visual_mode
        '<,'>s/\%V.*/\=l:output/
    else
        %delete 
        call append(0, split(l:output, "\n"))
    endif
    call winrestview(winsaved)
endfunction

nnoremap <leader>g1 :call OpenUnitTestAST(1)<CR>
nnoremap <leader>g2 :call OpenUnitTestAST(2)<CR>
nnoremap <leader>g3 :call OpenUnitTestAST(3)<CR>
nnoremap <leader>g4 :call OpenUnitTestAST(4)<CR>
nnoremap <leader>g5 :call OpenUnitTestAST(5)<CR>

nnoremap <leader>g6 :call PatchUnitTest(0)<CR>
vnoremap <leader>g6 :call PatchUnitTest(1)<CR>

