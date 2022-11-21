# Dálkové ovládání domácnosti
- ovladač
- vlastní model
- aplikace (pokud zbyde čas)

# Součástky a služby
- esp8266 / 4 relay esp8266
- mqtt server
- home assistent
- platformio

V mém projektu bych se chtěl věnovat dálkovému ovládání domácnosti pomocí esp8266.\
Projekt si představuji tak, že bych měl malý model žaluzií a k tomu bych sestrojil ovládání.\
Kód si představuji psát v jazyce c++.


# TO DO
- ~~koupit součástky (esp8266, relé...)~~
- ~~napsat kód na rozsvícení ledek (simulace motoru)~~
- ~~připojit k home assistantu (MQTT)~~
- naprogramovat:
  - žaluzie dolů
  - žaluzie nahoru
  - naklopení žaluzií
- všechno sestavit a zprovoznit

# Problémy
### ESP se mi nedaří připojit k wi-fi pomocí ESPHome
- Fix:
  - Připojoval jsem se k 5G WiFi, kterou esp nepodporuje
  - Stačilo se připojit k normální 2.4GHZ WiFi
### Nefunguje build
- Error:
  - Removing unused dependencies...
  Error: Traceback (most recent call last):
  File "C:\Users\kosna\.platformio\penv\lib\site-packages\platformio\__main__.py", line 102, in main\
    cli()  # pylint: disable=no-value-for-parameter\
  File "C:\Users\kosna\.platformio\penv\lib\site-packages\click\core.py", line 1130, in __call__   \ 
    return self.main(*args, **kwargs)\
  File "C:\Users\kosna\.platformio\penv\lib\site-packages\click\core.py", line 1055, in main        \
    rv = self.invoke(ctx)\
  File "C:\Users\kosna\.platformio\penv\lib\site-packages\platformio\cli.py", line 71, in invoke    \
    return super().invoke(ctx)\
  File "C:\Users\kosna\.platformio\penv\lib\site-packages\click\core.py", line 1657, in invoke      \
    return _process_result(sub_ctx.command.invoke(sub_ctx))\
  File "C:\Users\kosna\.platformio\penv\lib\site-packages\click\core.py", line 1404, in invoke      \
    return ctx.invoke(self.callback, **ctx.params)\
  File "C:\Users\kosna\.platformio\penv\lib\site-packages\click\core.py", line 760, in invoke       \
    return __callback(*args, **kwargs)\
  File "C:\Users\kosna\.platformio\penv\lib\site-packages\click\decorators.py", line 26, in new_func\
    return f(get_current_context(), *args, **kwargs)\
  File "C:\Users\kosna\.platformio\penv\lib\site-packages\platformio\run\cli.py", line 144, in cli\
    process_env(\
  File "C:\Users\kosna\.platformio\penv\lib\site-packages\platformio\run\cli.py", line 201, in process_env\
    result = {"env": name, "duration": time(), "succeeded": ep.process()}\
  File "C:\Users\kosna\.platformio\penv\lib\site-packages\platformio\run\processor.py", line 83, in process\
    install_project_env_dependencies(\
  File "C:\Users\kosna\.platformio\penv\lib\site-packages\platformio\package\commands\install.py", line 132, in install_project_env_dependencies\
    _install_project_env_libraries(project_env, options),\
  File "C:\Users\kosna\.platformio\penv\lib\site-packages\platformio\package\commands\install.py", line 204, in _install_project_env_libraries\
    _uninstall_project_unused_libdeps(project_env, options)\
  File "C:\Users\kosna\.platformio\penv\lib\site-packages\platformio\package\commands\install.py", line 280, in _uninstall_project_unused_libdeps\
    lm.uninstall(spec)\
  File "C:\Users\kosna\.platformio\penv\lib\site-packages\platformio\package\manager\_uninstall.py", line 29, in uninstall\
    return self._uninstall(spec, skip_dependencies)\
  File "C:\Users\kosna\.platformio\penv\lib\site-packages\platformio\package\manager\_uninstall.py", line 34, in _uninstall\
    pkg = self.get_package(spec)\
  File "C:\Users\kosna\.platformio\penv\lib\site-packages\platformio\package\manager\base.py", line 263, in get_package\
    spec = self.ensure_spec(spec)\
  File "C:\Users\kosna\.platformio\penv\lib\site-packages\platformio\package\manager\base.py", line 133, in ensure_spec\
    return spec if isinstance(spec, PackageSpec) else PackageSpec(spec)\
  File "C:\Users\kosna\.platformio\penv\lib\site-packages\platformio\package\meta.py", line 184, in __init__\
    self._parse(self.raw)\
  File "C:\Users\kosna\.platformio\penv\lib\site-packages\platformio\package\meta.py", line 291, in _parse\
    raw = parser(raw)\
  File "C:\Users\kosna\.platformio\penv\lib\site-packages\platformio\package\meta.py", line 316, in _parse_requirements\
    self.requirements = tokens[1].strip()\
  File "C:\Users\kosna\.platformio\penv\lib\site-packages\platformio\package\meta.py", line 231, in requirements\
    else semantic_version.SimpleSpec(str(value))\
  File "C:\Users\kosna\.platformio\penv\lib\site-packages\semantic_version\base.py", line 647, in __init__\
    self.clause = self._parse_to_clause(expression)\
  File "C:\Users\kosna\.platformio\penv\lib\site-packages\semantic_version\base.py", line 1043, in _parse_to_clause\
    return cls.Parser.parse(expression)\
  File "C:\Users\kosna\.platformio\penv\lib\site-packages\semantic_version\base.py", line 1063, in parse\
    raise ValueError("Invalid simple block %r" % block)\
  ValueError: Invalid simple block ''

  An unexpected error occurred. Further steps:\
  Verify that you have the latest version of PlatformIO using\
  `pip install -U platformio` command

  Try to find answer in FAQ Troubleshooting section\
  https://docs.platformio.org/page/faq/index.html

  Report this problem to the developers\
  https://github.com/platformio/platformio-core/issues

## Zdroje
1. MicroPython esp8266 tutorial:
   - https://docs.micropython.org/en/latest/esp8266/tutorial
2. Instalace, zprovoznění a add-ons home assistanta
   - https://www.youtube.com/watch?v=sVqyDtEjudk&ab_channel=TheHookUp
3. Připojení esp8266 k home assistantovi pomocí MQTT a ESPHome
   - https://youtu.be/NGCUfZhxY9U
