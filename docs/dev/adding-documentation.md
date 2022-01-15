# Documentation for Symbulation Documentation

This is a quick primer on how to document things within Symbulation.

Symbulation makes use of the Sphinx documentation system based off of XML
information gathered from Doxygen via a plugin named Breathe. This means
that Doxygen will automatically build documentation for anything written
in a C++ source file and Sphinx will be used to organize how that
documentation is displayed.

## Dependencies
You'll need to install some additional things to build the documentation locally.
If you make additions to the documentation, please build it locally to make sure that it is formatted well before making a pull request.

You need to install
- Doxygen
- Sphinx
- pip/pip3

For Homebrew users, that looks like this:
```
brew install sphinx
brew install doxygen
brew install pip3
```

You can then use `pip3` to install the rest of the requirements:
```
cd SymbulationEmp/docs
pip3 install -r requirements.txt
```

<!-- Old content, not sure what we need from it:
    Unix users
 
    a.  Install the python virtualenv, pip, gcc, and g++, cmake, bison,
        flex
 
        On recent Debian and Ubuntu this can be done with:
 
            sudo apt-get install python-virtualenv python-pip gcc g++ git gcovr cmake bison flex
 
        OS X users and others may need to download virtualenv first:
 
        curl -O <https://pypi.python.org/packages/source/v/virtualenv/virtualenv-1.11.6.tar.gz>
        tar xzf virtualenv* cd virtualenv-*; python2.7 virtualenv.py
        ../env; cd ..
 
    > [Mac ports](https://www.macports.org/) users on the OS X platform can
    >
    > :   install pip by execution from the command line:
    >
    >         sudo port install py27-pip
    >
    > [Homebrew](http://brew.sh/) users on the OS X platform will have
    > pip already installed


b.  Run the install-dependencies maketarget:
 
        make install-dependencies
 
    This will create a virtual python environment to use for Symbulation
    development. In this environment it will install:
    [Sphinx](http://sphinx-doc.org/),
    [Breathe](https://breathe.readthedocs.org/en/latest/), and
    [doxygen](http://www.stack.nl/~dimitri/doxygen/), packages we use to
    generate the documentation for Symbulation
 

## Building Documentation
Activate (or re-activate) the virtualenv:
 
        source third-party/env/bin/activate
 
    You can run this many times without any ill effects.
 
    (This puts you in the development environment.)
-->

## Building Documentation Locally

You are then ready to make your local documentation and run it:
```
make html
cd _build/html/
python3 -m http.server
```

## How to Comment for Doxygen Autodoc

Doxygen has an entire [documentation
section](https://www.stack.nl/~dimitri/doxygen/manual/docblocks.html) on
how to comment your code. We'll provide a trimmed version here so that
you can get started quickly.

Doxygen will examine all comments to determine if they are documentation
comments or just code comments. To make a documentation comment you must
add either an extra \* or /, depending on the kind of comment:

```cpp
/**
 * This is a documentation comment
 * across several lines
 *
 * This comment will be associated with the function immediately following.
*/
void somefunc(sometype param)
{

}

// this is a comment that doxygen will ignore
// note how it only has two leading slashes, like a normal comment
/// This is a comment that will be included in the documentation
/// Note the extra leading slash
/// Huzzah, documentation
```

````{note}
Doxygen requires a minimum of *three* triple slash'd
lines before a block is considered documentation:

```cpp
/// This line will be ignored
int somefunc() { return 5; }

///
/// This line will be included
///
void otherfunc() { ; }
```
````
If you wish to make a more visible comment block, e.g. a header for a
class, then you may do something like the following:

```cpp
/********************************************//**
* Here is some text inside a visible block
***********************************************/
```

```{note}
Doxygen will view this as any other documentation comment and
will not render it any differently than a 'normal' documentation
comment--it is simply more visible within the source code.
```

## How to include Doxygen's autodocs within Sphinx files

Through the use of the Breathe extension it is incredibly easy to
include Doxygen autodocs within a Sphinx documentation file.

Suppose we have a C++ implementation file name `lily.h` that has
inline comment documentation as detailed above and that `lily.h` is a
component of a module named `flowers` that was just created.

To document them, you must create a file within the Symbulation Library
documentation source to hold the module's documentation:

```cpp
    touch doc/library/flowers.md
```

Within `flowers.md` you can make an introduction to the module,
etc., and then add in the sphinx directives to include
auto-documentation. Your `flowers.md` file should look something
like the following:

```md
    # This is the flowers documentation!

    This is a very short introduction.

    ## lily.h

    ```{eval-rst}
    .. doxygenfile:: lily.h
       :project: Symbulation
    ```
```

When the docs are built Sphinx will automatically pull the available
documentation from Doxygen's XML files to construct the docs.

Additional directives exist to include auto-documentaiton from different
levels, the full breakdown of which is available within the [Breathe
Documentation](https://breathe.readthedocs.org/en/latest/directives.html).

## How to add docs to the Sphinx documentation

Sphinx is the system used to generate the developer guide and similar
reference documentation. A primer to using ReStructured Text, the markup
language used by Sphinx, can be found
[here](http://docutils.sourceforge.net/docs/user/rst/quickstart.html).
You can also look at any of the [.rst]{.title-ref} files in the
[doc/]{.title-ref} directory to get a feel for how thinks work.

New documents must be included in the `toctree` in the `index.md`
file for the directory the added file lives in. For example, if you add
`CowFacts.md` to the `CoolFacts/` directory you must add `CowFacts.md`
to the toctree found in `CoolFacts/CowFacts.md`:

```md
    # Cool Facts

    A bunch of cool facts!

    ```{toctree}
    AnteaterFacts
    BirdFacts
    CowFacts
    ```
```


