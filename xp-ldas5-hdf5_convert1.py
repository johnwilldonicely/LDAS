#!/usr/bin/python
"""
@author: Aidan Nickerson

This script extracts the cluster information from .kwik files into clubt, club and clulist for 
a given shank number and cluster group.
"""
import h5py
import numpy

# Has a command line interface use:
# python xs-ldas5-hdf5_convert1 [filename] -g [group] -s [shankno]
thisprog = "xs-ldas5-hdf5_convert1"


# getWaveforms=True):
def export_cluster_data(filename, shankno=0, cluster_group=2):
    """
    This function will extract the spike times and corresponding cluster IDs into two separate streams

    Parameters
    ----------
    filename = string filename
    shankno = integer the number in the HDFfile (previously used as shank, defaults to 0

    Returns
    ----------
    ((numpy array)cluster IDs, (numpy array)location)

    """
    # read hdf5 file
    print("input file: %s\n" % (filename))

    file = h5py.File(filename, 'r')
    g = file['/channel_groups/{}'.format(shankno)]

    # Spike timestamps (sample-number)
    timestamps = g['spikes/time_samples']

    # Spike cluster-ids
    clusterids = g['spikes/clusters/main']

    # alternatively, 0=noise, 1=MUA?, 3=unsorted
    goodclusters = [
        int(k) for k, v in g['clusters/main'].items() if v.attrs['cluster_group'] == cluster_group]

    print("Clusters:", goodclusters)

    # make a reference index to the "good" clusters
    refs = numpy.in1d(clusterids, goodclusters)

    # build the results structure, <timestamp><clusterids>
    result = (
        numpy.array(timestamps, dtype=numpy.int64)[refs],
        numpy.array(clusterids, dtype=numpy.int16)[refs],
        numpy.array(goodclusters, dtype=numpy.int16)
    )
    return result


def update_kwikfile(kwikfile, clubfile, outkwikfile=None, shankno=0, cluster_group=3):
    """
    this function will update the given kwikfile with a clubfile
    """

    if outkwikfile is not None:
        if outkwikfile != kwikfile:
            import shutil
            shutil.copy(kwikfile, outkwikfile)
            kwikfile = outkwikfile

    kwikf = h5py.File(kwikfile, 'r+')

    g = kwikf['/channel_groups/{}'.format(shankno)]
    clusterids = g['spikes/clusters/main']

    # alternatively, 0=noise, 1=MUA?, 3=unsorted
    goodclusters = [
        int(k) for k, v in g['clusters/main'].items() if v.attrs['cluster_group'] == cluster_group]

    refs = numpy.in1d(clusterids, goodclusters)
    clubf = numpy.fromfile(clubfile, dtype=numpy.int16)
    g['spikes/clusters/main'][:] = clubf[:]



from argparse import ArgumentParser
if __name__ == "__main__":

    parser = ArgumentParser(
        prog=thisprog, description="Extracts clubt, club and clulist from KlusterKwik files")
    subparser = parser.add_subparsers(help='subcommand options', dest="com")
    
    parser.add_argument('filename', help="KlusterKwik file to process")
    parser.add_argument(
        '-s', "--shank", help="Shank number to extract (default=0)", default=0, type=int)
    parser.add_argument(
        '-g', "--group", help="Cluster group to extract (default=2)", default=2, type=int)
    parser.add_argument('-c', "--club",
                        help="The club file to use, either to create with export or to update with with update (default=inputfile +'club')",
                        default=None)

   
    parser_export = subparser.add_parser(
        'export', help="Export the kwik file to club + clubt files")

    parser_update = subparser.add_parser(
        'update', help="update the kwik file with a club file")

    parser_update.add_argument(
        '-o', '--output', help="The output kwik file to use. if not defined it will use the input file + '.new.kwik'")

    result = parser.parse_args()

    filename = result.filename
    group = result.group
    shankno = result.shank
    if result.club is None:
        filename_extless = ".".join(filename.split(".")[:-1])
    else:
        filename_extless = result.club

    if result.com == "export":
        filename_extless = filename_extless.replace(".club", "")
        out = export_cluster_data(filename, shankno, cluster_group=group)
        
        
        out[0].tofile("{}.clubt".format(filename_extless))
        out[1].tofile("{}.club".format(filename_extless))
        out[2].tofile("{}.clulist".format(filename_extless))

    if result.com == "update":
        output = result.output
        if output == None:
            output = ".".join(filename.split(".")[:-1]) + ".new.kwik"
        
        if result.club is None:
            filename_extless += ".club"
        update_kwikfile(filename, filename_extless, output, shankno, group)
