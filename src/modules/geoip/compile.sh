cc -g3 -O2 -Wall -fPIC -c main.c -o geoip.o
cc -g3 -O2 -shared geoip.o -lGeoIP -o libgeoip.so
