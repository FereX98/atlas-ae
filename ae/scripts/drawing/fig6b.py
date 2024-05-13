import matplotlib.pyplot as plt
import pandas as pd
import matplotlib.lines as mlines

colors = [
    ["#3852A4", "#2D2B44", "#366897", "#859EB0"], # blue
    ["#7E479C", "#4D3763", "#624B78", "#8F709A"], # purple
    ["#00A99C", "#29503B", "#569F93", "#99C5AD"], # cyan
    ["#F37A20", "#C66228", "#F89D2D", "#FDBF4F"]  # yellow
]

percentiles = range(1, 100)

includes_paging = True
s='p99'

fs_latencies = []
atlas_latencies = []
aifm_latencies = []

if includes_paging:
    with open(r'/home/osdi/ae/results_baselines/fig6b/fastswap.csv') as file:
        fs_latencies = [int(line) for line in file]

with open(r'/home/osdi/ae/results/fig6b/atlas.csv') as file:
    atlas_latencies = [float(line) for line in file]

with open(r'/home/osdi/ae/results_baselines/fig6b/aifm.csv') as file:
    aifm_latencies = [int(line) for line in file]

paging_system = "Fastswap"

sample_percentile = [15,35,55,75,94,95,96,99]

if includes_paging:
    fs_latency_samples = [fs_latencies[item - 1] for item in sample_percentile]
atlas_latency_samples = [atlas_latencies[item - 1] for item in sample_percentile]
aifm_latency_samples = [aifm_latencies[item - 1] for item in sample_percentile]

fig, ax = plt.subplots()

if includes_paging:
    paging = mlines.Line2D([], [], linestyle='--',color="#4B0082", marker='s',markersize=15, label=paging_system)
    ax.plot(fs_latencies, percentiles, '--',  color="#4B0082",linewidth=3)
    ax.plot(fs_latency_samples, sample_percentile, linestyle='',marker = 's',markersize = 15,color="#4B0082",zorder=10)
atlas_line = mlines.Line2D([], [], linestyle='-',color='#366897', marker='x',markersize=15, label='Atlas')
ax.plot(atlas_latencies, percentiles, '-',  color="#366897",linewidth=3)
ax.plot(atlas_latency_samples, sample_percentile, linestyle='',marker = 'x',markersize = 15,color="#366897",zorder=10)
aifm_line = mlines.Line2D([], [], linestyle=':',color='slategray', marker='^',markersize=15, label='AIFM')
ax.plot(aifm_latencies, percentiles, ':',  color="slategray",linewidth=3)
ax.plot(aifm_latency_samples, sample_percentile, linestyle='',marker = '^',markersize = 15,color="slategray",zorder=10)


# ax.yaxis.set_major_formatter(in_gigabytes)
ax.set_ylabel('Accumulated Percentage', fontsize=24)
plt.yticks(fontsize=24)
plt.xticks(fontsize=24)
#ax.xaxis.set_major_formatter('{x}')
#ax.minorticks_off()
ax.set_xlabel('Latency (us)', fontsize=24)
#ax.set_ylim([0, 700])
ax.set_ylim([0, 100])
# set x to use log scale
ax.set_xscale('log')
#ax.set_xlim([10, 10**7])
#ax.set_xticks([100, 200, 300, 400, 500])
#ax.margins(0.1)

#plt.title(title, fontsize='x-large')
if includes_paging:
    handles,labels = ax.get_legend_handles_labels()
    handles = [atlas_line,paging,aifm_line]
    labels = ['Atlas',paging_system,'AIFM']
else:
    handles,labels = ax.get_legend_handles_labels()
    handles = [atlas_line,aifm_line]
    labels = ['Atlas','AIFM']
plt.legend(handles, labels, fontsize=25, loc=(0.23, 0.1), frameon=False)

fig.tight_layout()
plt.savefig(f'/home/osdi/ae/scripts/drawing/fig6b.pdf')

#plt.show()