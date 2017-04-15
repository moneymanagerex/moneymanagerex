#!/bin/bash
cat /etc/*release | grep ^NAME | sed -e "s/NAME=//" -e 's/^"//' -e 's/"$//'