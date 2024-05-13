for i in {0..47};do
sudo cpufreq-set -c $i -g userspace;
sudo cpufreq-set -c $i -f 2800000;
done