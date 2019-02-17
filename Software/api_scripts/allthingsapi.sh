#!/bin/sh

url="https://api.allthingstalk.io";
authorization="${ALLTHINGSAPI_AUTHORIZATION}";
device_id="${ALLTHINGSAPI_DEVICE_ID}";

method="$1";
asset="$2";
from="$3"
to="$4"
page="$5"

today="$(date --iso-8601)"
yesterday="$(date --iso-8601 -d "yesterday")"

curlGet()
{
    case "$from" in
        # Get the latest known value of the asset.
        "latest")
            curl -X ${method} "${url}/device/${device_id}/asset/${asset}/state" \
                 -H "Authorization: Bearer ${authorization}"
            ;;
        # Get the aggregated value.
        "aggregated")
            curl -X ${method} "${url}/device/${device_id}/asset/${asset}/states?resolution=day&from=${from}&to=${to}" \
                 -H "Authorization: Bearer ${authorization}"
            ;;
        # All other cases will evaluate to use a range from yesterdays date to today and display page 0.
        *)
            curl -X ${method} "${url}/device/${device_id}/asset/${asset}/states?from=${yesterday}&to=${today}&page=0" \
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
    usage
    exit 1
fi

