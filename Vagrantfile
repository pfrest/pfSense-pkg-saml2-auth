Vagrant.configure("2") do |config|
  config.vm.guest = :freebsd
  config.vm.box = ENV['FREEBSD_VERSION'] || "freebsd/FreeBSD-14.1-STABLE"
  #config.vm.box_version = "2024.10.03"
  config.ssh.shell = "sh"
  config.vm.synced_folder ".", "/vagrant", id: "vagrant-root", disabled: true
  #config.vm.base_mac = "080027D14C68"

  config.vm.provision "shell", inline: <<-SHELL
    whoami
    pkg update -y
    pkg upgrade -y
    pkg install -y python311
    pkg install -y php82-composer
    pkg install -y gitup
    gitup ports
    su vagrant -c "python3.11 -m ensurepip"
    su vagrant -c "python3.11 -m pip install jinja2"
  SHELL
  config.vm.provider "virtualbox" do |vb|
    vb.customize ["modifyvm", :id, "--memory", "1024"]
    vb.customize ["modifyvm", :id, "--cpus", "2"]
    vb.customize ["modifyvm", :id, "--hwvirtex", "on"]
    vb.customize ["modifyvm", :id, "--audio", "none"]
    vb.customize ["modifyvm", :id, "--nictype1", "virtio"]
    vb.customize ["modifyvm", :id, "--nictype2", "virtio"]
  end
end