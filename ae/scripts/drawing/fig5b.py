import matplotlib.pyplot as plt
import numpy as np
import matplotlib.lines as mlines

colors = [
    ["#3852A4", "#2D2B44", "#366897", "#859EB0"], # blue
    ["#7E479C", "#4D3763", "#624B78", "#8F709A"], # purple
    ["#00A99C", "#29503B", "#569F93", "#99C5AD"], # cyan
    ["#F37A20", "#C66228", "#F89D2D", "#FDBF4F"]  # yellow
]

cycles_per_us = 2800
include_fastswap = True
include_aifm = True

percentiles = np.array(range(1, 100))

samples = np.array(range(0, 10))
samples_idx = [15,35,55,74,94,95,96,99]
#print("samples_idx = ",samples_idx)

def numbers_from_file(filename):
    with open(filename) as file:
        return [float(line.strip()) for line in file]

# osdi 24
if include_fastswap:
    fs_latencies = numbers_from_file(r'/home/osdi/ae/results_baselines/fig5b/fastswap.csv')
    fs_latencies = [item for item in fs_latencies]
at_latencies = numbers_from_file(r'/home/osdi/ae/results/fig5b/atlas.csv')
at_latencies = [item for item in at_latencies]
ai_latencies = numbers_from_file(r'/home/osdi/ae/results_baselines/fig5b/aifm.csv')
ai_latencies = [item for item in ai_latencies]

paging_system = "Fastswap"

fig, ax = plt.subplots()

fs_samples = []
if include_fastswap:
    for i in range(len(samples_idx)):
        fs_samples.append(fs_latencies[samples_idx[i]-1])
at_samples = []
for i in range(len(samples_idx)):
    at_samples.append(at_latencies[samples_idx[i]-1])
ai_samples = []
for i in range(len(samples_idx)):
    ai_samples.append(ai_latencies[samples_idx[i]-1])
percentiles_sample = []
for i in range(len(samples_idx)):
    percentiles_sample.append(percentiles[samples_idx[i]-1])

if include_fastswap:
    ax.plot(fs_latencies, percentiles, '--', label=paging_system, color="#4B0082", markersize = 15, linewidth=3)
    ax.plot(fs_samples, percentiles_sample, linestyle='',marker = 's',markersize = 15,color="#4B0082",zorder=10)
    fs_line = mlines.Line2D([], [], linestyle='--',color="#4B0082", marker='s',markersize=15, label=paging_system)

ax.plot(at_latencies, percentiles, '-', label='Atlas', color="#366897", markersize = 15, linewidth=3)
ax.plot(at_samples, percentiles_sample, linestyle='',marker = 'x',markersize = 15,color="#366897",zorder=10)
atlas_line = mlines.Line2D([], [], linestyle='-',color="#366897", marker='x',markersize=15, label='Atlas')

if include_aifm:
    ax.plot(ai_latencies, percentiles, ':', label='AIFM', color="slategray", markersize = 15, linewidth=3)
    ax.plot(ai_samples, percentiles_sample, linestyle='',marker = '^',markersize = 15,color="slategray",zorder=10)
    aifm_line = mlines.Line2D([], [], linestyle=':',color="slategray", marker='^',markersize=15, label='AIFM')

# ax.yaxis.set_major_formatter(in_gigabytes)
ax.set_ylabel(r'Accumulated Percentage', fontsize=24)
plt.yticks(fontsize=24)
plt.xticks(fontsize=24)
#ax.xaxis.set_major_formatter('{x}')
#ax.minorticks_off()
ax.set_xlabel('Latency (us)', fontsize=24)
ax.set_ylim([0, 100])
#ax.set_xlim([10, 100])
ax.set_xscale('log')
#ax.set_xlim([0, 500])
ax.set_yticks([0, 20, 40, 60, 80, 100])
y_ticks=['0','20%','40%','60%','80%','100%']
ax.set_yticklabels(y_ticks,fontsize=24)
#ax.set_xticks([20, 50, 100, 200])
#ax.set_xticklabels(['20', '50', '100', '200'],fontsize=24)
#ax.xaxis.set_major_locator(ticker.MultipleLocator(40))
#ax.margins(0.1)

#plt.title(title, fontsize='x-large')
handles,labels = ax.get_legend_handles_labels()
handles = [atlas_line]
labels = ['Atlas']
if include_fastswap:
    handles.append(fs_line)
    labels.append(paging_system)
if include_aifm:
    handles.append(aifm_line)
    labels.append('AIFM')
plt.legend(handles,labels,fontsize=20,loc=(0.3,0.4),frameon=False)
fig.subplots_adjust(bottom=0.18,top = 0.95,left=0.25,right=0.95)

# fig.tight_layout()
plt.savefig(r'/home/osdi/ae/scripts/drawing/fig5b.pdf')
