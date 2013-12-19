rm src/js/pebble-js-app.js 2> /dev/null
cat src/js/*.js >> src/js/pebble-js-app.js
pebble build || { exit $?; }
if [ "$1" = "install" ]; then
    pebble install --logs
fi
