sudo apt-get install ncftp doxygen graphviz;
pushd ${TRAVIS_BUILD_DIR};
pushd docs;

pip install -r requirements.txt;
make;

ncftpput -R -v -u $FTP_USER  -p $FTP_PASSWORD  static.syscop.de acados_website/ _build/*;
popd;
popd;
