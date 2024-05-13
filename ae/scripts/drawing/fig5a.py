import matplotlib.pyplot as plt
import numpy as np

colors = [
    ["#3852A4", "#2D2B44", "#366897", "#859EB0"], # blue
    ["#7E479C", "#4D3763", "#624B78", "#8F709A"], # purple
    ["#00A99C", "#29503B", "#569F93", "#99C5AD"], # cyan
    ["#F37A20", "#C66228", "#F89D2D", "#FDBF4F"]  # yellow
]

workload = 'Web App'

percentile = 90

local_in_gbs = 10

all_local = False

offered_load = True

cpu_freq_in_mhz = 2800

def numbers_from_file(filename):
    with open(filename) as file:
        return [float(line.strip()) for line in file]

# 90p latency osdi24
atlas_throughput = numbers_from_file("/home/osdi/ae/results/fig5a/atlas-t")
atlas_latency = numbers_from_file("/home/osdi/ae/results/fig5a/atlas-l")
fs_throughput = numbers_from_file("/home/osdi/ae/results_baselines/fig5a/fastswap-t")
fs_latency    = numbers_from_file("/home/osdi/ae/results_baselines/fig5a/fastswap-l")
aifm_throughput = numbers_from_file("/home/osdi/ae/results_baselines/fig5a/aifm-t")
aifm_latency = numbers_from_file("/home/osdi/ae/results_baselines/fig5a/aifm-l")

atlas_latency = [item / cpu_freq_in_mhz for item in atlas_latency]
fs_latency = [item / cpu_freq_in_mhz for item in fs_latency]
aifm_latency = [item / cpu_freq_in_mhz for item in aifm_latency]

fig, ax = plt.subplots()

plt.plot(fs_throughput, fs_latency, 's--', label='Fastswap', color="#4B0082", markersize = 15, linewidth=3)
plt.plot(atlas_throughput, atlas_latency, 'x-', label='Atlas', color='#366897', markersize = 15, linewidth=3)
plt.plot(aifm_throughput, aifm_latency, '^:', label='AIFM', color='slategray', markersize = 15, linewidth=3)


# ax.yaxis.set_major_formatter(in_gigabytes)
ax.set_ylabel(str(percentile) + 'th Percentile Latency (us)', fontsize=20)
plt.yticks(fontsize=24)
plt.xticks(fontsize=24)
#ax.set_xticks([0, 0.1, 0.2, 0.3, 0.4, 0.5, 0.6])
#ax.set_yticks([200, 400, 600, 800, 1000])
#ax.xaxis.set_major_formatter('{x}')
#ax.minorticks_off()
ax.set_xlabel('Throughput (MOPS)', fontsize=24)
ax.set_yscale('log')
#ax.set_ylim([100, 400000])
#ax.margins(0.1)

#plt.title(title, fontsize='x-large')
lgnd = plt.legend(fontsize=18, loc=(0.5,0.43), fancybox=True, framealpha=0.01)
#plt.legend(fontsize=18, loc='best')

fig.tight_layout()
plt.savefig(f'/home/osdi/ae/scripts/drawing/fig5a.pdf')

#plt.show()