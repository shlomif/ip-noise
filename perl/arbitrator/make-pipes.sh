#!/bin/bash

if [ ! -e $HOME/ip-noise ] ; then
	mkdir $HOME/ip-noise
fi

if [ ! -e $HOME/ip-noise/pipes ] ; then
	mkdir $HOME/ip-noise/pipes
fi

if [ ! -e $HOME/ip-noise/pipes/to_arb ] ; then
	mkfifo $HOME/ip-noise/pipes/to_arb
fi

if [ ! -e $HOME/ip-noise/pipes/from_arb ] ; then
	mkfifo $HOME/ip-noise/pipes/from_arb
fi

