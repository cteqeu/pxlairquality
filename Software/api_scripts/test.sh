#!/bin/sh

url="https://api.allthingstalk.io";
authorization="${ALLTHINGSAPI_AUTHORIZATION}";
device_id="${ALLTHINGSAPI_DEVICE_ID}";

method="$1";
asset="$2";
from="$3"
to="$4"
page="$5"

curlGet()
{
    case "$from" in
        "latest")
            curl -X ${method} "${url}/device/${device_id}/asset/${asset}/state" \
                 -H "Authorization: Bearer ${authorization}"
            ;;
        "arregated")
            curl -X ${method} "${url}/device/${device_id}/asset/${asset}/states?resolution=day&from=${from}&to=${to}" \
                 -H "Authorization: Bearer ${authorization}"
            ;;
        *)
            curl -X ${method} "${url}/device/${device_id}/asset/${asset}/states?from=${from}&to=${to}&page=${page}" \
                 -H "Authorization: Bearer ${authorization}"
            ;;
    esac

}

usage()
{
    echo "To use this script first export the environment variables:"
    echo "    - ALLTHINGSAPI_DEVICE_ID={your device id}"
    echo "    - ALLTHINGSAPI_AUTHORIZATION={your authorization key}"
    echo "Or edit these variables directly in the script."
}

if [ "$#" -ge 1 ]; then
    case "$1" in
        "GET")
            curlGet
            ;;
        "POST")
            echo "Not yet implemented"
            ;;
        *)
            usage
            exit 1
    esac
else
    echo "No arguments supplied"
    exit
fi

