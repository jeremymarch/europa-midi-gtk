# europa-midi-gtk

europa-midi-gtk is a midi librarian for the Roland Jupiter 6 synth with Europa mod written in C with gtk2.  

This is an archive of old code.  The code in the original github commit was updated March 6, 2005.  

To build it you'll need:

- sudo apt-get install gtk2.0
- sudo apt-get install build-essentials libgtk2.0-dev
- sudo apt-get install libasound2-dev
- sudo apt install mariadb-server
- sudo apt install libmariadb-dev-compat
- sudo mysql_secure_installation

Launch the program with arguments for your mysql/mariadb:

./europa -h localhost -u dbusername -p dbpassword -d dbname


