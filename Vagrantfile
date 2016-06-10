# -*- mode: ruby -*-
# vi: set ft=ruby :

Vagrant.configure(2) do |config|
  config.vm.hostname = "ifs-factal-generator"
  config.vm.box = "ubuntu/trusty64"
  
  # runs as root within the VM
  config.vm.provision "shell", inline: %q{
  
    set -e  # stop on error
    
    apt-get update
    DEBIAN_FRONTEND=noninteractive apt-get upgrade -q -y    # grub upgrade warnings mess with the terminal
    apt-get -q -y install vim gcc build-essential ntp unattended-upgrades 

  }
  
  config.vm.provision "shell", run: "always", inline: %q{
  
    set -e  # stop on error
    
    echo ''
    echo '-----------------------------------------------------------------'
    echo 'After "vagrant ssh", use:'
    echo '   cd /vagrant '
    echo '   gcc *.c -lm -o ifs '
    echo '   ./ifs '
    echo '-----------------------------------------------------------------'
    echo ''
  }
  
end
