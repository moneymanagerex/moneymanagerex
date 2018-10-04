#!/bin/bash
USERNAME="mm"

echo "Money Manager Ex: Starting with UID:GID $LOCAL_USER_ID:$LOCAL_GROUP_ID"
usermod -u $LOCAL_USER_ID $USERNAME
usermod -g $LOCAL_GROUP_ID $USERNAME
export HOME=/home/$USERNAME

exec gosu $USERNAME "/home/mm/moneymanagerex/build/src/mmex" $@
