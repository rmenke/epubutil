DOC_VERSION := $(shell sed -ne '/projectnumber/ !d; s/.*&\#160;//; s/<.*//p' doc/html/index.html)

commit:
	git commit -m '$(DOC_VERSION)'

git-tag:
	@case `git describe --dirty --always` in *-dirty) echo "git: uncommitted changes"; exit 1;; esac
	git tag -a -f -m "doc-$(DOC_VERSION)" "doc-$(DOC_VERSION)"
