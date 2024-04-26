#if !defined(ADG_FPRINTF)
#   define ADG_FPRINTF(...) fprintf(__VA_ARGS__)
#endif
#if !defined(ADG_FPRINTF_COLOR)
#   define ADG_FPRINTF_COLOR(print_func, fp, color, fmt, ...) \
        print_func(fp, color fmt, __VA_ARGS__)
#endif
#if !defined(ADG_COLOR_RESET)
#   define ADG_COLOR_RESET    "\u001b[0m"
#endif
#if !defined(ADG_COLOR_HEADING1)
#define ADG_COLOR_HEADING1 "\u001b[1;37m"
#endif
#if !defined(ADG_COLOR_HEADING2)
#define ADG_COLOR_HEADING2 "\u001b[1;34m"
#endif
#if !defined(ADG_COLOR_LONGOPT)
#define ADG_COLOR_LONGOPT  "\u001b[1;32m"
#endif
#if !defined(ADG_COLOR_SHORTOPT)
#define ADG_COLOR_SHORTOPT "\u001b[1;36m"
#endif
#if !defined(ADG_COLOR_ARG)
#define ADG_COLOR_ARG      "\u001b[1;36m"
#endif
#if !defined(ADG_COLOR_ERROR)
#   define ADG_COLOR_ERROR "\u001b[1;31m"
#endif
