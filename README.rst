Introduction
============

squid_nufw_helper is an external helper for the Squid external ACL scheme.
It provides Sigle Sign On (SSO) based on the NuFW firewall suite.

Usage
=====

The usage for the program is ::

    squid_nufw_helper -f <configuration_file> -u -c 1000

Options:

- -f points to the config file
- -u means squid_nufw_helper should return the found user ID
- -c <number> tells squid_nufw_helper to process that many connexions
  before dying (and having Squid spawn a new helper process)
- -a passes the source port in htons() to resolve byte ordering problems
  that can appear. -a is NOT needed on squid 2.6 on AMD64.


Any bug to be reported to authors, so that we improve quality of this software!
And, we are waiting for patches, too!

