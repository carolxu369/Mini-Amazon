#!/bin/bash
python3 ./Amazon/manage.py makemigrations
python3 ./Amazon/manage.py migrate
res="$?"
while [ "$res" != "0" ]
do
    sleep 3;
    python3 ./Amazon/manage.py migrate
    res="$?"
done
