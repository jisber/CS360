ge=/home/jplank/cs360/labs/lab4/Gradescript-Examples
rm -f testmain.c tf1.c tf2.c tf3.c tf4.c tf0.c s1_header.h s2_header.h
rm -f testmain.o tf1.o tf2.o tf3.o tf4.o tf0.o testexec testlib.a
cp $ge/allfile.c testmain.c
sed 's/first/first/' $ge/oneline-5.c > tf0.c
sed 's/first/second/' $ge/oneline-1.c > tf1.c
sed 's/first/third/' $ge/oneline-4.c > tf2.c
sed 's/first/fourth/' $ge/oneline-2.c > tf3.c
sed 's/first/fifth/' $ge/oneline-3.c > tf4.c
gcc -c testmain.c
touch -t 01011100.30 testmain.c
touch -t 01011100.35 testmain.o
gcc -c tf0.c
touch -t 01011101.30 tf0.c
touch -t 01011101.35 tf0.o
gcc -c tf1.c
touch -t 01011102.30 tf1.c
touch -t 01011102.35 tf1.o
gcc -c tf2.c
touch -t 01011103.30 tf2.c
touch -t 01011103.35 tf2.o
gcc -c tf3.c
touch -t 01011104.30 tf3.c
touch -t 01011104.35 tf3.o
gcc -c tf4.c
touch -t 01011105.30 tf4.c
touch -t 01011105.35 tf4.o
gcc -o testexec testmain.c tf1.c tf2.c tf3.c tf4.c tf0.c -lm
touch -t 01011106.35 testexec
touch -t 01011100.25 testmain.o
rm -f tf0.o
touch -t 01011103.25 tf2.o
touch -t 01011104.25 tf3.o
touch -t 01011105.25 tf4.o
if /home/jplank/cs360/labs/lab4/fakemake $ge/056.fm; then
  ./testexec
fi
