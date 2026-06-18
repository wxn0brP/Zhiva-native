#include "init_script.h"

const char *zhiva_init_script()
{
    return R"(
        document.addEventListener('DOMContentLoaded', () => {
            const titleObserver = new MutationObserver((mutations) => {
                mutations.forEach((mutation) => {
                    if (mutation.type === 'attributes' && mutation.attributeName === 'title' && mutation.target.title) {
                        window.zhiva_setWindowTitle(mutation.target.title);
                    }
                });
            });
            const titleElement = document.querySelector('title');
            if (titleElement) {
                titleObserver.observe(titleElement, { attributes: true });
                if (document.title) {
                    zhiva_setWindowTitle(document.title);
                }
            }

            document.addEventListener('click', (e) => {
                const anchor = e.target.closest('a');
                if (anchor && anchor.target === '_blank') {
                    e.preventDefault();
                    zhiva_openExternal(anchor.href);
                }
            }, true);

            window.addEventListener("message", (event) => {
                if (event.data && event.data.type === "open-link") {
                    zhiva_openExternal(event.data.url);
                }
            });
        });
    )";
}
