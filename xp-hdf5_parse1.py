#!/usr/local/bin/python3


# script for reading HDF5 output from MultiChannelSystems recordings - combined timestamps and waveforms for detected events
# EXAMPLES
#	xp-hdf5_parse1.py count infile.h5		- get the max-channel-number (number of channels minus one)
# 	xp-hdf5_parse1.py export 2 infile.h5	- export timestamps and waveforms for channel 2
#	xp-hdf5_parse1.py metadata infile.h5	- print metadata to screen
#	xp-hdf5_parse1.py shape infile.h5		-
#   xp-hdf5_parse1.py --recording 1 attributes - print the attributes for recording 1
#   xp-hdf5_parse1.py attributes            - print the file attributes
#   xp-hdf5_parse.py -h                     - displays the help information


import h5py
import argparse
from pathlib import Path
import numpy as np

def get(args):
    h5 = h5py.File(args.filename)
    prefix = Path(args.filename).name
    path = 'Data/Recording_{recording}/{type}Stream/Stream_{stream}'
    timestamp_grp = h5[path.format(recording=args.recording, stream=args.stream, type="TimeStamp")]
    timestamp = timestamp_grp['TimeStampEntity_{}'.format(args.entity_no)].value.astype(np.int64)

    data_grp= h5[path.format(recording=args.recording, stream=args.stream, type="Segment")]
    data = data_grp['SegmentData_{}'.format(args.entity_no)].value.astype(np.float32)

    # output the timestamps
    timestamp.tofile("{}.{}.i64.timestamp.dat".format(prefix,args.entity_no))
    # output the waveforms, transposed
 #   data = data.T
 #   data.tofile("{}.{}.f32.dat".format(prefix,args.entity_no))

def count(args):
    h5 = h5py.File(args.filename)
    recording = 0 if args.recording is None else args.recording
    path = 'Data/Recording_{recording}/{type}Stream/Stream_{stream}'
    timestamp_grp = h5[path.format(recording=recording, stream=args.stream, type="TimeStamp")]
    print(len(timestamp_grp))

def display_structure(h5):
    output = []
    try:
        for k in list(h5.keys()):
            children = display_structure(h5[k])
            if len(children)>0:
                o = {k:children}
            else:
                o = k
            output.append(o)
    except:
        pass
    return output

def shape(args):
    h5 = h5py.File(args.filename)
    recording = 0 if args.recording is None else args.recording
    path = 'Data/Recording_{recording}/{type}Stream/Stream_{stream}'
    data_grp= h5[path.format(recording=recording, stream=args.stream, type="Segment")]
    data = data_grp['SegmentData_{}'.format(0)].value.astype(np.float32)
    print(data.shape)

def display_structure_cmd(args):
    print(display_structure(h5py.File(args.filename)))


import sys
def get_meta(args):
    h5 = h5py.File(args.filename)
    path = 'Data/Recording_{recording}/{type}Stream/Stream_{stream}/SourceInfoChannel'
    recording = 0 if args.recording is None else args.recording
    info_dataset = h5[path.format(recording=recording, stream=args.stream, type="Segment")]
    if args.output is None:
        f = sys.stdout
    else:
        f = open(args.output, "w+")


    f.write("\t".join(info_dataset.dtype.names))
    f.write("\n")
    def decode(x):
        if isinstance(x, bytes):
            return x.decode()
        return str(x)

    for x in info_dataset:

        f.write("\t".join(map(decode,x)))
        f.write("\n")


def get_attr(args):
    h5 = h5py.File(args.filename)
    if args.recording is None:
        path = "Data"
    else:
        path = 'Data/Recording_{recording}/'.format(recording=args.recording)

    if args.path is not None:
        path = args.path

    attrs = h5[path].attrs

    if args.output is None:
        f = sys.stdout
    else:
        f = open(args.output, "w+")

    def decode(x):
        if isinstance(x, bytes):
            return x.decode()
        return str(x)
    for k, v in attrs.items():
        f.write( k + "\t" + decode(v) + "\n")


# COMMAND LINE INTERFACE


parser = argparse.ArgumentParser(description='Read Spike data file')
parser.add_argument('--recording', type=int, help='The recording number', default=None)
parser.add_argument('--stream', type=int, help="The stream number", default=0)
subparser = parser.add_subparsers()
parser.add_argument('filename', type=str)

get_parser = subparser.add_parser("export", help="Export the data into seperate dat files")
get_parser.add_argument("entity_no", type=int)
get_parser.set_defaults(func=get)

count_parser = subparser.add_parser("count", help="Count the number of events to stdout")
count_parser.set_defaults(func=count)

structure_parser = subparser.add_parser("display", help="Display the hdf5 structure")
structure_parser.set_defaults(func=display_structure_cmd)

structure_parser = subparser.add_parser("shape", help="Display the shape of the waveform matrix")
structure_parser.set_defaults(func=shape)

meta_parser = subparser.add_parser("metadata", help="Export the metadata into a tab-separated file")
meta_parser.add_argument("--output", type=str, default=None, help="An output file. If not specified printed to stdout")
meta_parser.set_defaults(func=get_meta)

attr_parser = subparser.add_parser("attributes", help="Export the attributes into a tab-separated file")
attr_parser.add_argument("--output", type=str, default=None, help="An output file. If not specified printed to stdout")
attr_parser.add_argument("--path", type=str, default=None, help="The HDF5 path to dump (overrides others)")
attr_parser.set_defaults(func=get_attr)

if __name__ == "__main__":
    args = parser.parse_args()
    args.func(args)
