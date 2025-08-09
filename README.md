# Compiling:

This project uses make to simplify the compilation. You can tweak the compilation flags by modifying the `Makefile` file.
Running `make all` from the root directory compiles all the source files into an executable called `plotgen.out` located in the the `bin` folder.

# Running:

If it doesn't exist already, create a folder named `data` in the root directory, then simply run the generated executable. This should create a file with a `.plot` extension in the `data` folder.
That file describes the generated plot.

# Plotting

Run `make <filename>.plot` from the root directory or directly run the `plot.py` script with the path to the `.plot` file as an argument. <br>
If a `.venv` environment exists in the root directory, `make <filename>.plot` will use that, otherwise it will use the global python environment. <br>

##

Note that the `matplotlib` python module is used for plotting. You can install it globally with `pip install matplotlib`. The more recommended method however is to create a virtual python environment specific to the project.
You can do that by running the following commands:
```
$> python -m venv .venv
$> .venv/bin/pip install matplotlib
```

# Example:

## Commands

```
$> mkdir data
$> make all
$> ./bin/plotgen.out
$> make comparison.plot
```
## Output

<img width="640" height="480" alt="plotexample" src="https://github.com/user-attachments/assets/b84d7123-3ccc-40c4-acd2-47a047a1ce4e" />

