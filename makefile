SUBDIRS := backend/compile_server backend/oj_server backend/crawler

.PHONY: all $(SUBDIRS) output clean frontend

all: $(SUBDIRS) frontend

$(SUBDIRS):
	@echo "==> Building $@"
	@$(MAKE) -C $@ --no-print-directory

frontend:
	@echo "==> Building frontend"
	@if [ -d "frontend" ] && command -v npm >/dev/null 2>&1; then \
		cd frontend && npm install && npm run build && \
		mkdir -p ../backend/oj_server/wwwroot && \
		cp -r dist/* ../backend/oj_server/wwwroot/; \
	else \
		echo "Skipping frontend build (npm not found or frontend dir missing)"; \
	fi

output: all
	@echo "==> Preparing output bundle"
	@mkdir -p output/compile_server
	@mkdir -p output/oj_server
	@mkdir -p output/crawler
	@cp -rf backend/compile_server/compile_server output/compile_server/
	@if [ -d backend/compile_server/temp ]; then cp -rf backend/compile_server/temp output/compile_server/; fi
	@cp -rf backend/oj_server/conf output/oj_server/
	@cp -rf backend/oj_server/wwwroot output/oj_server/
	@cp -rf backend/oj_server/questions output/oj_server/
	@cp -rf backend/oj_server/oj_server output/oj_server/
	@cp -rf backend/crawler/contest_crawler output/crawler/
	@mkdir -p output/logs
	@mkdir -p output/data

clean:
	@for d in $(SUBDIRS); do \
		echo "==> Cleaning $$d"; \
		$(MAKE) -C $$d clean --no-print-directory || exit $$?; \
	done
	@rm -rf output
