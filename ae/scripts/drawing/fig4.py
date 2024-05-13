import matplotlib.pyplot as plt
import numpy as np

def read_input(app):
    systems = ["atlas", "fastswap", "aifm"]
    ratios = ['13', '25', '50', '75', '100']
    ret = [[] for i in range(3)]
    path = [f"/home/osdi/ae/results/fig4/{app}", f"/home/osdi/ae/results_baselines/fig4/{app}", f"/home/osdi/ae/results_baselines/fig4/{app}"]
    for i,s in enumerate(systems):
        for r in ratios:
            try:
                with open(f"{path[i]}/{s}-{r}", "r") as file:
                    ret[i].append(float(file.read()))
            except FileNotFoundError:
                # If the file does not exist, set the variable to 1
                ret[i].append(1.0)
    return ret

# DataFrame
# 13%, 25%, 50%, 75%, all local
data1 = read_input('df')
data1 = np.array(data1) / 1000000.

# Web Service
# 13% first
data2 = read_input('ws')

num_ops = 50

for i in range(len(data2)):
    for j in range(len(data2[i])):
        data2[i][j] = num_ops / data2[i][j]

# Metis WC
data3 = read_input('mwc')
data3 = np.array(data3)/1000.

# Aspen TC
data4 = read_input('atc')

# MCD-CL
# 13%, 25%, 50%, 75% all local
# this is throughput
data5 = read_input('mcd-cl')

num_ops = 250*1000*1000*4

for i in range(len(data5)):
    for j in range(len(data5[i])):
        data5[i][j] = num_ops / data5[i][j]

# GPR
data6 = read_input('gpr')
data6 = np.array(data6)/1000000. + 43.768022

# MCD-U
# 13%, 25%, 50%, all local
# this is throughput
# [Atlas, Fastswap, AIFM]^T
data7 = read_input('mcd-u')
data7 = np.array(data7)

num_ops = 250*1000*1000*4

data7 = num_ops / data7

# MPVC
# 13%, 25%, 50%, all local
# [Atlas, Fastswap, AIFM]^T
data8 = read_input('mpvc')
data8 = np.array(data8) / 1000000.

data = [data1, data2, data3, data4, data5, data6, data7, data8]

fig = plt.figure(constrained_layout=False)
subfigs = fig.subfigures(2, 4)
f1 = subfigs[0,0].subplots(1,1)
# tc_fig = subfigs[0,1].subfigures(2, 1, height_ratios=[1, 1], hspace=0)
f2 = subfigs[0,1].subplots(1, 1)
#wd_fig = subfigs[0,2].subplots(2,1)
#com_wd = wd_fig.flat[0]
#com_wd_1 = wd_fig.flat[1]
f3 = subfigs[0,2].subplots(1,1)
f4 = subfigs[0,3].subplots(1,1)
f5 = subfigs[1,0].subplots(1,1)
f6 = subfigs[1,1].subplots(1,1)
f7 = subfigs[1,2].subplots(1,1)
f8 = subfigs[1,3].subplots(1,1)

x = np.zeros(5,dtype=float)
x[0] = 0
x[1] = 1
x[2] = 2
x[3] = 3
x[4] = 4
x_label = ['13','25','50', '75', '100']

#subplot 1
width=0.2
xticks_font = 55
ylabel_font = 55
xlabel_font = 55
title_font = 55
yticks_font = 55
legend_font =55

f1.bar(x - width,data5[0][0:5],color = "#366897",ec = 'k', ls = '-',width=width,zorder=10)
f1.bar(x        ,data5[1][0:5],color = "#8F709A",ec = 'k', ls = '-',width=width,zorder=10)
f1.bar(x + width,data5[2][0:5],color = "slategrey",ec = 'k', ls = '-',width=width,zorder=10)
f1.bar(x,[0,0,0,0,0],tick_label = x_label)
#f1.axhline(y=data5[0][3], linestyle='-', color = "#6495ED",linewidth=4,zorder=20)
#f1.axhline(y=data5[1][3], linestyle=':', color = "#FF8C00",linewidth=4,zorder=20)
f1.axhline(y=data5[1][4], linestyle='--', color = "black",linewidth=4,zorder=20)
f1.set_ylabel('Execution Time (s)', fontsize=ylabel_font)
# plt.axline(linestyle='--',label="All Local",color = 'k')
# f1.set_ylabel('Execution Time (s)', fontsize=30)
#f1.set_ylim(0,800)
#f1.set_xlabel('Local Memory Ratio (%)', fontsize=30)
f1.set_title("(a) Memcached-CL", fontsize=title_font)
f1.set_xticklabels(x_label,fontsize=xticks_font)
#y_label = ['0','','200','','400','','600','','800']
#f1.set_yticklabels(y_label,fontsize=30)
f1.tick_params(axis='y', which='major', labelsize=yticks_font)

f2.bar(x - width,data7[0][0:5],color = "#366897",ec = 'k', ls = '-',width=width,zorder=10)
f2.bar(x        ,data7[1][0:5],color = "#8F709A",ec = 'k', ls = '-',width=width,zorder=10)
f2.bar(x + width,data7[2][0:5],color = "slategrey",ec = 'k', ls = '-',width=width,zorder=10)
f2.bar(x,[0,0,0,0,0],tick_label = x_label)
f2.axhline(y=data7[1][4], linestyle='--', color = "black",linewidth=4,zorder=20)
#f2.set_ylabel('Execution Time (s)', fontsize=30)
# plt.axline(linestyle='--',label="All Local",color = 'k')
# f2.set_ylabel('Execution Time (s)', fontsize=30)
#f2.set_ylim(0,800)
#f2.set_xlabel('Local Memory Ratio (%)', fontsize=30)
f2.set_title("(b) Memcached-U", fontsize=title_font)
f2.set_xticklabels(x_label,fontsize=xticks_font)
#y_label = ['0','','200','','400','','600','','800']
#f2.set_yticklabels(y_label,fontsize=30)
f2.tick_params(axis='y', which='major', labelsize=yticks_font)

f3.bar(x - width,data6[0][0:5],color = "#366897",ec = 'k', ls = '-',width=width,zorder=10)
f3.bar(x        ,data6[1][0:5],color = "#8F709A",ec = 'k', ls = '-',width=width,zorder=10)
f3.bar(x + width,data6[2][0:5],color = "slategrey",ec = 'k', ls = '-',width=width,zorder=10)
f3.bar(x,[0,0,0,0,0],tick_label = x_label)
#f3.axhline(y=data6[0][3], linestyle='-', color = "#6495ED",linewidth=4,zorder=20)
#f3.axhline(y=data6[1][3], linestyle=':', color = "#FF8C00",linewidth=4,zorder=20)
f3.axhline(y=data6[1][4], linestyle='--', color = "black",linewidth=4,zorder=20)
# plt.axline(linestyle='--',label="All Local",color = 'k')
# f3.set_ylabel('Execution Time (s)', fontsize=30)
#f3.set_ylim(0,800)
#f3.set_xlabel('Local Memory Ratio (%)', fontsize=30)
f3.set_title("(c) GraphOne PR", fontsize=title_font)
f3.set_xticklabels(x_label,fontsize=xticks_font)
#y_label = ['0','','200','','400','','600','','800']
#f3.set_yticklabels(y_label,fontsize=30)
f3.tick_params(axis='y', which='major', labelsize=yticks_font)

f4.bar(x - width,data4[0][0:5],color = "#366897",ec = 'k', ls = '-',width=width,zorder=10)
f4.bar(x        ,data4[1][0:5],color = "#8F709A",ec = 'k', ls = '-',width=width,zorder=10)
f4.bar(x + width,data4[2][0:5],color = "slategrey",ec = 'k', ls = '-',width=width,zorder=10)
f4.bar(x,[0,0,0,0,0],tick_label = x_label)
f4.axhline(y=data4[1][4], linestyle='--', color = "black",linewidth=4,zorder=20)
# plt.axline(linestyle='--',label="All Local",color = 'k')
#f4.set_ylabel('Execution Time (s)', fontsize=30)
# f4.set_ylim(0,800)
#f4.set_xlabel('Local Memory Ratio (%)', fontsize=30)
f4.set_title("(d) Aspen TC", fontsize=title_font)
f4.set_xticklabels(x_label,fontsize=xticks_font)
y_label = ['0','','30','','60','','90']
f4.tick_params(axis='y', which='major', labelsize=yticks_font)

f5.bar(x - width,data3[0][0:5],color = "#366897",ec = 'k', ls = '-',width=width,zorder=10)
f5.bar(x        ,data3[1][0:5],color = "#8F709A",ec = 'k', ls = '-',width=width,zorder=10)
f5.bar(x + width,data3[2][0:5],color = "slategrey",ec = 'k', ls = '-',width=width,zorder=10)
f5.bar(x,[0,0,0,0,0],tick_label = x_label)
#f5.axhline(y=data3[0][3], linestyle='-', color = "#6495ED",linewidth=4,zorder=20)
#f5.axhline(y=data3[1][3], linestyle=':', color = "#FF8C00",linewidth=4,zorder=20)
f5.axhline(y=data3[1][4], linestyle='--', color = "black",linewidth=4,zorder=20)
#f5.set_xlabel('Local Memory Ratio (%)', fontsize=30)
f5.set_ylabel('Execution Time (s)', fontsize=ylabel_font)
f5.set_xlabel('Local Memory Ratio (%)', fontsize=xlabel_font)
f5.set_title("(e) Metis WC", fontsize=title_font)
f5.set_xticklabels(x_label,fontsize=xticks_font)
#y_label = ['0','','40','','80','','120']
#f5.set_yticklabels(y_label,fontsize=30)
f5.tick_params(axis='y', which='major', labelsize=yticks_font)

f6.bar(x - width,data8[0][0:5],color = "#366897",ec = 'k', ls = '-',width=width, label='Atlas',zorder=10)
f6.bar(x        ,data8[1][0:5],color = "#8f709A",ec = 'k', ls = '-',width=width, label='Fastswap',zorder=10)
f6.bar(x + width,data8[2][0:5],color = "slategrey",ec = 'k', ls = '-',width=width, label='AIFM',zorder=10)
f6.bar(x,[0,0,0,0,0],tick_label = x_label)
#f6.axhline(y=data1[0][3], linestyle='-', label="All Local Atlas", color = "#6495ED",linewidth=4,zorder=20)
# f6.plot(x, [data_com[0][3],data_com[0][3],data_com[0][3]],linestyle='-',linewidth=4,label="All Local FastSwap",marker = 'x',color = "#6495ED",zorder=20)
#f6.axhline(y=data1[1][3], linestyle=':', label="All Local FastSwap", color = "#FF8C00",linewidth=4,zorder=20)
f6.axhline(y=data8[1][4], linestyle='--', label="All Local", color = "black",linewidth=4,zorder=20)
# plt.axline(linestyle='--',label="All Local",color = 'k')
#f6.set_ylabel('Execution Time (s)', fontsize=30)
#f6.set_ylim(0,500)
# f6.set_xlabel('Local Memory Ratio (%)', fontsize=30)
#f6.set_xlabel('Local Memory Ratio (%)', fontsize=30)
f6.set_title("(f) Metis PVC", fontsize=title_font)
f6.set_xlabel('Local Memory Ratio (%)', fontsize=xlabel_font)
#y_label = ['0','100','200','300','400','500']
f6.set_xticklabels(x_label,fontsize=xticks_font)
f6.tick_params(axis='y', labelsize=yticks_font)

f7.bar(x - width,data1[0][0:5],color = "#366897",ec = 'k', ls = '-',width=width,zorder=10)
f7.bar(x        ,data1[1][0:5],color = "#8F709A",ec = 'k', ls = '-',width=width,zorder=10)
f7.bar(x + width,data1[2][0:5],color = "slategrey",ec = 'k', ls = '-',width=width,zorder=10)
f7.bar(x,[0,0,0,0,0],tick_label = x_label)
#f7.axhline(y=data1[0][3], linestyle='-', label="All Local Atlas", color = "#6495ED",linewidth=4,zorder=20)
# f7.plot(x, [data_com[0][3],data_com[0][3],data_com[0][3]],linestyle='-',linewidth=4,label="All Local FastSwap",marker = 'x',color = "#6495ED",zorder=20)
#f7.axhline(y=data1[1][3], linestyle=':', label="All Local FastSwap", color = "#FF8C00",linewidth=4,zorder=20)
f7.axhline(y=data1[1][4], linestyle='--', color = "black",linewidth=4,zorder=20)
# plt.axline(linestyle='--',label="All Local",color = 'k')
# f7.set_ylabel('Execution Time (s)', fontsize=30)
#f7.set_ylim(0,500)
# f7.set_xlabel('Local Memory Ratio (%)', fontsize=30)
f7.set_xlabel('Local Memory Ratio (%)', fontsize=xlabel_font)
f7.set_title("(g) DataFrame", fontsize=title_font)
#y_label = ['0','100','200','300','400','500']
f7.set_xticklabels(x_label,fontsize=xticks_font)
f7.tick_params(axis='y', labelsize=yticks_font)

f8.bar(x - width,data2[0][0:5],color = "#366897",ec = 'k', ls = '-',width=width,zorder=10)
f8.bar(x        ,data2[1][0:5],color = "#8f709A",ec = 'k', ls = '-',width=width,zorder=10)
f8.bar(x + width,data2[2][0:5],color = "slategrey",ec = 'k', ls = '-',width=width,zorder=10)
f8.axhline(y=data2[1][4], linestyle='--', color = "black",linewidth=4,zorder=20)
f8.bar(x,[0,0,0,0,0],tick_label = x_label)
# f8.axhline(y=data2[0][3], linestyle='-', color = "#6495ED",linewidth=4,zorder=20)
# f8.axhline(y=data2[1][3], linestyle=':', color = "#FF8C00",linewidth=4,zorder=20)
# plt.axline(linestyle='--',label="All Local",color = 'k')
# f8.set_ylabel('Execution Time (s)', fontsize=30)
#f8.set_ylim(0,1000)
# f8.set_xlabel('Local Memory Ratio (%)', fontsize=30)
f8.set_xlabel('Local Memory Ratio (%)', fontsize=xlabel_font)
f8.set_title("(h) Web Service", fontsize=title_font)
# f8.set_xticklabels(x_label,fontsize=30)
#y_label = ['','','','','6000','','8000']
f8.set_xticklabels(x_label,fontsize=xticks_font)
f8.tick_params(axis='y', labelsize=yticks_font)
#f8.set_yticklabels(y_label,fontsize=30)
# f8.tick_params(axis='both', labelsize=30)

# fig.tight_layout()
leg = fig.legend(loc="lower center", bbox_to_anchor=(0.5, -0.15), ncol=4,fontsize=legend_font)
# leg.set_in_layout(False)
fig.set_size_inches(50, 19)
fig.subplots_adjust(top = 0.9, bottom=0.1,left=0.1,right=0.9)
# subfigs[0,0].subplots_adjust(top = 0.9, bottom=0.1,left=0.2,right=0.95)
fig.subplots_adjust(wspace=0.8)
#subfigs[0,1].subplots_adjust(hspace=0.05)
#subfigs[0,2].subplots_adjust(hspace=0.05)
fig.savefig("/home/osdi/ae/scripts/drawing/fig4.pdf",bbox_inches="tight")
#plt.show()
