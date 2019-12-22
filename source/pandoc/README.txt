# this release: https://github.com/jgm/pandoc/releases/tag/2.7.2
# file: pandoc-2.7.2-linux.tar.gz

source=/opt/LDAS/source/pandoc/pandoc-2.7.2-linux.tar.gz
dest=/opt/pandoc/

sudo tar xvzf $source --strip-components 1 -C $dest
