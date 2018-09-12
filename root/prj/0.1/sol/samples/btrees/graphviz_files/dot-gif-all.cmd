@SET PATH=%PATH%;C:\Program Files (x86)\Graphviz2.38\bin

@rem dot -Tps gr1.gv -o gr1.gif
@rem dot -Tps %1 -o %1.gif

@rem for %%f in (*.*) do @echo GraphViz-ing %%f
for %%f in (*.gv) do dot -Tgif %%f -o %%f.gif
