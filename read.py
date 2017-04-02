#########################################
#   Use like this:
# python read.py path/to/filename.h5
#########################################

import h5py
import numpy as np
import matplotlib
import matplotlib.pyplot as plt
import sys

filename = sys.argv[1] 

matplotlib.rcParams.update({'font.size': 8})

#extracts a variable->column_number map from dataset
def columns_map(dset):
	cols = dset.attrs["columns"].replace(' ','').split(",")
	return {cols[k]: k for k in range(len(cols))}

#plots prarticular domain&range combination (with uncerts if available)
#and labels it with name
def plot_metric(domain_name, range_name):
	cmap = columns_map(dset)
	if domain_name not in cmap or range_name not in cmap: return
	domain_col = cmap[domain_name]
	range_col = cmap[range_name]
	if (range_name+"_uncert") in cmap:
		uncert_col = cmap[range_name+"_uncert"]
		plt.errorbar(dset[...,domain_col], dset[...,range_col], 
			yerr=dset[...,uncert_col])
	else:
		plt.plot(dset[...,domain_col], dset[...,range_col])
	#plt.xlabel(domain_name)
	ll = domain_name
	if ll == "%count":
		ll = "acceptance"
	plt.title("%s vs %s" % (range_name, ll))


f = h5py.File(filename, "r")

nrows = 2
ncols = 4

for metric in f.keys():
	
	dset = f[metric + "/results"]

	domain = "val_max"
	if dset.attrs['varied_min'] != 0: 
		domain = "val_min"

	plt.suptitle(metric)

	#row 1
	plt.subplot(nrows, ncols, 1)
	plot_metric(domain, '%count')

	plt.subplot(nrows, ncols, 2)
	plot_metric(domain, 'position')

	plt.subplot(nrows, ncols, 3)
	plot_metric(domain, 'SnR')

	plt.subplot(nrows, ncols, 4)
	plot_metric(domain, 'resolution')

	#row 2
	plt.subplot(nrows, ncols, 5)
	plot_metric('%count', 'signal')

	plt.subplot(nrows, ncols, 6)
	plot_metric('%count', 'background')

	plt.subplot(nrows, ncols, 7)
	plot_metric('%count', 'SnR')

	plt.subplot(nrows, ncols, 8)
	plot_metric('%count', 'resolution')

	#plt.tight_layout()
	plt.subplots_adjust(right=.97, left=.06, top=.9, bottom=.05, wspace=.5, hspace=.22)
	plt.savefig("%s-%s.png" % (filename, metric), dpi=600)
	plt.clf()
