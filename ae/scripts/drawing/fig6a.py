import matplotlib.pyplot as plt
import pandas as pd

colors = [
    ["#3852A4", "#2D2B44", "#366897", "#859EB0"], # blue
    ["#7E479C", "#4D3763", "#624B78", "#8F709A"], # purple
    ["#00A99C", "#29503B", "#569F93", "#99C5AD"], # cyan
    ["#F37A20", "#C66228", "#F89D2D", "#FDBF4F"]  # yellow
]

def numbers_from_file(filename):
    with open(filename) as file:
        return [float(line.strip()) for line in file]

# 90p latency osdi24
atlas_throughput = numbers_from_file("/home/osdi/ae/results/fig6a/atlas-t")
atlas_latency = numbers_from_file("/home/osdi/ae/results/fig6a/atlas-l")
paging_throughput = numbers_from_file("/home/osdi/ae/results_baselines/fig6a/fastswap-t")
paging_latency    = numbers_from_file("/home/osdi/ae/results_baselines/fig6a/fastswap-l")
aifm_thruput = numbers_from_file("/home/osdi/ae/results_baselines/fig6a/aifm-t")
aifm_latency = numbers_from_file("/home/osdi/ae/results_baselines/fig6a/aifm-l")

atlas_throughput = [l / 1000000 for l in atlas_throughput]

percentile = 90

paging_system = "Fastswap"

fig, ax = plt.subplots()

plt.plot(paging_throughput, paging_latency, 's--', label=paging_system, color="#4B0082", markersize = 15, linewidth=3)
plt.plot(atlas_throughput, atlas_latency, 'x-', label='Atlas', color='#366897', markersize = 15, linewidth=3)
plt.plot(aifm_thruput, aifm_latency, '^:', label='AIFM', color='slategrey', markersize = 15, linewidth=3)


# ax.yaxis.set_major_formatter(in_gigabytes)
ax.set_ylabel(str(percentile) + 'th Percentile Latency (us)', fontsize=20)
plt.yticks(fontsize=24)
plt.xticks(fontsize=24)
#ax.xaxis.set_major_formatter('{x}')
ax.set_xlabel('Throughput (mops)', fontsize=24)
#ax.set_xticks([0, 0.5, 1.0, 1.5, 2.0])
#ax.set_xticklabels([0, 0.5, 1.0, 1.5, 2.0])
ax.set_yscale('log')
ax.minorticks_off()
#ax.set_ylim([0, 700])
#ax.margins(0.1)

#plt.title(title, fontsize='x-large')
plt.legend(fontsize=20, loc=(0.35, 0.4), framealpha=0.01)

fig.tight_layout()
plt.savefig(f'/home/osdi/ae/scripts/drawing/fig6a.pdf')

#plt.show()