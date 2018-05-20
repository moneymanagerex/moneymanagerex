#!/usr/bin/env bash

declare -A users commits

while IFS=$'|' read -r user year; do
  [[ ${users[$user]} != *$year* ]] && users[$user]+=" $year"
  commits[$user]=$((${commits[$user]}+1))
done < <(git log --all --author-date-order --reverse --date=format:%Y --pretty="format:%aN <%aE>|%ad")

for u in "${!users[@]}"; do echo -e "${users[$u]}\t$u\t${commits[$u]}"; done | sort
