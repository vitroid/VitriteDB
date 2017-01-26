###Vitrite online database
##Requrements
#C libraries
Berkeley DB 4.3 (brew install berkeley-db4)
OpenSSL (brew install openssl) It is deplecated but it is used for hashing.
mysql (brew install mysql)
povray (brew install povray)

#Perl libraries
XML::Generator (sudo cpanm XML::Generator)
Class::HPLOO::Base () To install DBD
DBD::mysql


##Notes
A "/etc/my.cnf" from another install may interfere with a Homebrew-built
server starting up correctly.

To connect:
    mysql -uroot

To have launchd start mysql at login:
    ln -sfv /usr/local/opt/mysql/*.plist ~/Library/LaunchAgents
Then to load mysql now:
    launchctl load ~/Library/LaunchAgents/homebrew.mxcl.mysql.plist
Or, if you don't want/need launchctl, you can just run:
    mysql.server start

#Apache hack
An update on Sep 10, 2014 quetly disabled CGI execution...F*ck
Uncomment the following line in /etc/apache2/httpd.conf

    LoadModule cgi_module libexec/apache2/mod_cgi.so


#Dump the DB on the old server

    mysqldump --single-transaction --databases vitrite -umatto -pvitrite > /tmp/dumped_vitrite_database

#Create the database on the new server

    mysql> CREATE USER 'matto'@'localhost' IDENTIFIED BY 'vitrite';
    mysql> GRANT ALL PRIVILEGES ON * . * TO 'matto'@'localhost';
    %  mysql -umatto -p < ~/Downloads/dumped_vitrite_database 

* imageを自動生成する部分が動いているかどうかわからない。
  * mysqlにimageが生成済みかどうかの情報があると思う。
* MySQLの中身が不明。今となってはDBエンジンなんかつかうより、pythonの辞書を使ったほうが簡単。
* 公開


