Update 6-Feb-2011
Version 4.0

extensions:

a) Two Scheduling policies

- Wait for an idle node to become available
	- Wait for a node with Cpref of task (The initial version of method putinsuspensionqueue should work in this case)
	- Wait for any idle node with sufficient area to become free (need to update putinsuspensionqueue)


The method CheckSuspensionQueue is changed to wait for any idle node with sufficient area rather than Cpref.

-Call it policy-3