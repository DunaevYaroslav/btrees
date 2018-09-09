@SET PATH=%PATH%;C:\Program Files (x86)\Graphviz2.38\bin

@rem dot -Tps gr1.gv -o gr1.png
@rem dot -Tps %1 -o %1.png

@rem for %%f in (*.*) do @echo GraphViz-ing %%f
for %%f in (*.gv) do dot -Tpng %%f -o %%f.png
