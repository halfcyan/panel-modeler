# Bash completion for panel-modeler
_panel_modeler() {
    local cur prev
    cur="${COMP_WORDS[COMP_CWORD]}"
    prev="${COMP_WORDS[COMP_CWORD - 1]}"

    if [[ "${prev}" == "--climate" ]]; then
        COMPREPLY=()
        return 0
    fi

    if [[ "${cur}" == -* ]]; then
        COMPREPLY=( $(compgen -W '--climate --version --help -h help' -- "${cur}") )
    else
        COMPREPLY=( $(compgen -f -- "${cur}") )
    fi
}
complete -F _panel_modeler panel-modeler
