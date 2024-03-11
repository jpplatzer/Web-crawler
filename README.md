# Web-crawler

An example application I developed to peform multi-threaded crawling of a website's child pages. You can use its pluggable processing class to develop you own custom page processing on website pages.

The Thread_pool class in the include directory handles the thread management.

The application use the curl library to handle the http/https website communication.

You'll need to install the curl development package to build the application.

You can install GNU package on Debian/Ubuntu Linux systems using.

sudo apt install libcurl4-gnutls-dev 
