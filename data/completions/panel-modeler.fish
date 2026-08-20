# Fish completion for panel-modeler
complete -c panel-modeler -f
complete -c panel-modeler -n '__fish_is_nth_token 1' -l climate -d 'Look up climate data for coordinates or an address'
complete -c panel-modeler -n '__fish_is_nth_token 1' -l version -d 'Print the version'
complete -c panel-modeler -n '__fish_is_nth_token 1' -l help -d 'Show help'
complete -c panel-modeler -n '__fish_is_nth_token 1' -s h -d 'Show help'
complete -c panel-modeler -n '__fish_is_nth_token 1' -a help -d 'Show help'
complete -c panel-modeler -n '__fish_is_nth_token 2' -a '(__fish_seen_subcommand_from --climate)' -d 'latitude,longitude or address'
complete -c panel-modeler -n '__fish_is_nth_token 1' -a '(__fish_complete_suffix .csv)' -d 'Input CSV'
complete -c panel-modeler -n '__fish_is_nth_token 2' -a '(__fish_complete_suffix .csv)' -d 'Output CSV'
