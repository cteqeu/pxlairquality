#!/bin/sh

method="$7";
device_id="$1";
asset="$2";
authorization="$6";
from="$3"
to="$4"
page="$5"
url="https://api.allthingstalk.io";


curlGet ()
{
    curl -X GET "${url}/device/${device_id}/asset/${asset}/states?from=${from}&to=${to}&page=${page}" \
         -H "Authorization: Bearer ${authorization}"
}

if [ "$#" -ge 1 ]; then
    curlGet
else
    echo "No arguments supplied"
    exit
fi
