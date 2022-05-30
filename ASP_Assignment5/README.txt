Name: Mohit Palliyil Sathyaseelan

Subject: ASP



Assignment 5 :
Folder contains 
1) char_driver.c
2) userapp.c
3) Makefile
4) Readme.txt


---------------------------------------------------------------------------------------------------------------------------------------------------------

To execute the assignment5
either run the make file 
or 
run following commands in the respective folder.

make –C /usr/src/linux-headers-$(uname –r) M=$PWD modules
sudo insmod char_driver.ko dev_nr=3
sudo gcc userapp.c -o userapp
sudo ./userapp 2
//write or read commands
sudo rmmod char_driver


for any queries please do get back