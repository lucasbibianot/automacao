#!/usr/bin/env bash
for var in "$@"
do
    nvar=$(echo $var | tr -d ",[]")
    filename=$(basename $nvar)
    modulename="${filename%.*}"
    echo "Módulo: $modulename"
    cmd="/usr/sbin/semodule -i $filename"
    $cmd
    if [ $? -eq 0 ]
    then
      echo "ok"
    else
      echo "erro"
    fi
done