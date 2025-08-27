whoami
pkg update -y
pkg upgrade -y
pkg install -y python311
pkg install -y php82-composer
pkg install -y gitup
gitup ports
su vagrant -c "python3.11 -m ensurepip"
su vagrant -c "python3.11 -m pip install jinja2"