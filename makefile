SUBDIRS := compile_server oj_server crawler

.PHONY: all $(SUBDIRS) output clean

all: $(SUBDIRS)

$(SUBDIRS):
	@echo "==> Building $@"
	@$(MAKE) -C $@ --no-print-directory

output: all
	@echo "==> Preparing output bundle"
	@mkdir -p output/compile_server
	@mkdir -p output/oj_server
	@mkdir -p output/crawler
	@cp -rf compile_server/compile_server output/compile_server/
	@if [ -d compile_server/temp ]; then cp -rf compile_server/temp output/compile_server/; fi
	@cp -rf oj_server/conf output/oj_server/
	@cp -rf oj_server/css output/oj_server/
	@cp -rf oj_server/questions output/oj_server/
	@cp -rf oj_server/template_html output/oj_server/
	@cp -rf oj_server/wwwroot output/oj_server/
	@cp -rf oj_server/oj_server output/oj_server/
	@cp -rf crawler/contest_crawler output/crawler/
	@mkdir -p output/logs
	@mkdir -p output/data

clean:
	@for d in $(SUBDIRS); do \
		echo "==> Cleaning $$d"; \
		$(MAKE) -C $$d clean --no-print-directory || exit $$?; \
	done
	@rm -rf output
