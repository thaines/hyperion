#!/bin/bash
find . -type d -exec chmod 740 {} \;
find . -type f -exec chmod 640 {} \;

chmod 740 *.sh
chmod 740 eos/*.sh
chmod 740 ares/*.sh
chmod 740 ares/dep/*.sh
chmod 740 docs/cyclops/*.sh
chmod 740 dist/*.sh
