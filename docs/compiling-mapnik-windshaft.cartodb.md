# How to compile mapnik, node-mapnik and friends
## Intro
`mapnik` is a wonderful library for rendering tiles, written mostly in C++. Then there's `node-mapnik`, aka `mapnik` (inside node the module is called `mapnik` but it is actually a binding so they are distinct things). Then a number of other things that indirectly depend on the `mapnik` library. This is a guide of typical procedures for the adventurous willing to modify and get it running.

## An overview of the dependencies
This is a graph of the dependencies, just to give you an idea of what's behind the scenes:

![node-mapnik dependencies](http://g.gravizo.com/source/custom_mark10?https%3A%2F%2Fraw.githubusercontent.com%2FCartoDB%2Fmapnik%2Fcompiling-mapnik-windshaft-cartodb%2Fdocs%2Fcompiling-mapnik-windshaft.cartodb.md)
<details> 
<summary></summary>
custom_mark10
digraph G {
        "windshaft-cartodb" -> windshaft;
        windshaft -> "node-mapnik";
        windshaft -> "tilelive-mapnik";
        windshaft -> "tilelive-bridge";
        windshaft -> "abaculus";
        "tilelive-mapnik" -> "node-mapnik";
        "tilelive-bridge" -> "node-mapnik";
        abaculus -> "node-mapnik";
        subgraph cluster0 {
                label="binding, c++, .so libraries, need to work together"
                "node-mapnik" -> mapnik;
        }
        subgraph cluster1 {
            label="c and c++ stuff"
            mapnik -> boost,
            mapnik -> libcairo,
            mapnik -> libpng,
            mapnik -> "..."
        }
}
custom_mark10
</details>

Notes:

* if you are curious about the actual dependencies of a node module, just run `npm list` after installing all its dependencies with `npm install`.
* `npm` allows for the usage of different versions for the same dependency. E.g: `windshaft` might directly depend on node-mapnik ver `1.4.15-cdb7` whilst `abacus` can depend on node-mapnik `1.4.15-cdb6` (and thus `windshaft` would depend on two different versions, one directly and the other indirectly).
* It is advisable to stick to just one version, the latest stable, though.
* npm dependency resolution can depend on the order of execution of commands. In case of trouble just delete the `node_modules/` fracking directoy.


## Development mode
This is the mode in which you want to use the same VM for compiling and testing it, hopefully reducing the modify-compile-test turnaround cycle in which you don't really care about having the exact versions or a reproducible build but being able to see the effect of your changes beyond the unit tests.

### Compile mapnik 2.3.x.cartodb
First, install it's zillion dependencies. There are two or three options here. I wouldn't care much about them as long as it compiles (the `./configure` is pretty smart).
https://github.com/CartoDB/mapnik/blob/2.3.x.cartodb/INSTALL.md

(another option is to look into the travis script and figure out what it does).

```
git clone git@github.com:CartoDB/mapnik.git
git checkout 2.3.x.cartodb
cd mapnik
./configure
make
make test
sudo make install
```

Some notes:

* don't be too greedy with the JOBS parameter, it'll fail if it runs out of memory.
* `sudo make install` get's the _sdk_ installed into your local VM
* there are ways to avoid `sudo` but not worth the effort IMHO.

### Compile node-mapnik cdb-1.x
```
git clone git@github.com:CartoDB/node-mapnik.git
git checkout cdb-1.x
npm install --build-from-source
npm test
sudo npm link --build-from-source
```

Some notes:

* `npm link` is the npm way of letting you work with development versions. It creates a link to your freshly backed node module in a place where it can be referenced later on (in my machine it is `/usr/lib/node_modules/` along with other global modules).
* there are ways of avoiding `sudo` for npm. But I leave that as an exercise to the reader.

### Compile windshaft
```
git clone https://github.com/cartodb/windshaft
npm install --build-from-source
npm link mapnik --build-from-source
sudo npm link
```

Some notes:

* `npm link mapnik` is what makes it use the previously linked node-mapnik module, so that instead of resolving dependencies and grabbing it from the internets, it picks your freshly backed version.
* again, `sudo npm link` is what makes it available to play this trick on other modules that depend on `windshaft`


### Compile windshaft-cartodb
```
git clone https://github.com/cartodb/windshaft-cartodb
cd /home/ubuntu/www/node-windshaft/current
npm install --build-from-source
npm link windshaft
```

And the you can run it with: `node app.js development`


### Some general notes

* You might want to compile it with debugging symbols (`gdb` and friends) and log traces:
```
./configure DEBUG=yes ENABLE_LOG=yes DEFAULT_LOG_SEVERITY=debug
```

* Sometimes it can become tricky even to know what you're running. Don't hesitate to add the typical `printf("hello honey, I'm here\n");` or equivalent (take a look at `MAPNIK_LOG_DEBUG()` macro usages).


## Release mode
This is the mode in which you need to care of:

- reproducible builds
- tagging
- using precise versions of the dependencies (c++ compiler toolchain, libcairo, boost, etc)
- bumping the version of the new dependency (windshaft-cartodb that windshaft that depends on node-mapnik that depends on mapnik sdk and so forth).

You need to go bottom-up, from `mapnik` up to `windshaft-cartodb`.

Start cloning this repo and check the [README](https://github.com/CartoDB/node-mapnik-packaging-recipes)
```
git clone git@github.com:CartoDB/node-mapnik-packaging-recipes.git
```

### Compile and upload the mapnik SDK
Basically you need to follow the [README](https://github.com/CartoDB/node-mapnik-packaging-recipes/tree/master/mapnik)

What it does:

- grab a ton of dependencies
- compile some of them
- setup env vars and all the stuff to produce a clean build with those dependencies
- package everything into a reusable SDK that you can use to compile/link against.

Before starting, if you peek into the scripts you'll see it is using this other repo:

https://github.com/cartodb/mapnik-packaging

and it basically builds whatever it is in the mapnik branch [2.3.x.cartodb](https://github.com/CartoDB/mapnik-packaging/blob/30fb9e995fe4a10182abb764ca7e4d578ab64dfa/build.sh#L253) (to date)

So after your changes get the green light:


* merge them into `2.3.x.cartodb`
* run the instructions [here](https://github.com/CartoDB/node-mapnik-packaging-recipes/tree/master/mapnik)

If everything goes right, it will produce a tarball with a name following this convention:
```
mapnik-${arch}-sdk-${git-describe}.tar.bz2
```

where `${git-describe}` is the result of running `git describe` on the `HEAD` of `2.3.x.cartodb`. It identifies the current commit based on the latest tag (for more info see [git-describe](https://git-scm.com/docs/git-describe) documentation. E.g:

```
[rtorre@rtorre-cartodb mapnik (2.3.x.cartodb)]$ git describe
v2.2.0-699-g996ea2d
```

and the resulting file is (in the host machine):
```
$ find -name mapnik\*.tar.bz2
./opt/mapnik-packaging/osx/out/dist/mapnik-linux-sdk-v2.2.0-699-g996ea2d.tar.bz2
```

In order to make that sdk available to dependent packages, it needs to be uploaded to S3, to this [bucket](https://console.aws.amazon.com/s3/home?region=us-east-1#&bucket=cartodb-node-binary&prefix=dist/dev/)

Note there are sdk's and buckets for mac and redhat as well.

TODO: I don't know how hard it is to automate the upload but for the moment I just uploaded it manually and granted read permissions to everyone.

### Compile and upload node-mapnik
Next step is to compile and upload `node-mapnik`, the node bindings for `mapnik` library, against the freshly backed mapnik sdk.

* Take a look at https://github.com/CartoDB/node-mapnik-packaging-recipes/tree/master/node-mapnik
* First thing note that the script [package](https://github.com/CartoDB/node-mapnik-packaging-recipes/blob/master/node-mapnik/scripts/package) uses the repo https://github.com/CartoDB/node-mapnik , branch `cdb-1.x`.
* You should go there, bump the version number and make it point to the new SDK. You can take this [commit](https://github.com/CartoDB/node-mapnik/commit/0a786c462b43a9dfd01f0fbd728277224f48fc27) as an example. The `HASH` is obtained from `git describe`, identifying the source of mapnik sdk.
* You should also tag the new version:
```
$ git tag -a '1.4.15-cdb8' -m 'Upgrade mapnik sdk to 699-g996ea2d'
$ git push --tags origin  1.4.15-cdb8
```
and probably mark it as the _latest release_ in github.

* Then follow the rest of the instructions [there](https://github.com/CartoDB/node-mapnik-packaging-recipes/tree/master/node-mapnik)

It shall produce the following lines at the end of the script:
```
[mapnik] Package appears valid
node-pre-gyp info ok
```

Once you're done it should have run the tests and created an artifact called `{node_abi}-{platform}-{arch}.tar.gz` (see the node-mapnik [package.json](https://github.com/CartoDB/node-mapnik/blob/12669bdd80c3910cf45197441a031057c0f699a1/package.json#L14)). E.g:
```
$ find opt/node-mapnik/build -name \*.tar.gz
opt/node-mapnik/build/stage/mapnik/v1.4.15-cdb8/node-v11-linux-x64.tar.gz
```

This should be uploaded to this S3 [bucket](https://console.aws.amazon.com/s3/home?region=us-east-1#&bucket=cartodb-node-binary&prefix=mapnik/) to its corresponding folder. You can use the provided script (on the vm):
```
# keys omitted
$ AWS_ACCESS_KEY_ID="" AWS_SECRET_ACCESS_KEY="" ./scripts/publish
node-pre-gyp info it worked if it ends with ok
node-pre-gyp info using node-pre-gyp@0.5.25
node-pre-gyp info using node@0.10.26 | linux | x64
[mapnik] Success: published to https://cartodb-node-binary.s3.amazonaws.com/mapnik/v1.4.15-cdb8/node-v11-linux-x64.tar.gz
node-pre-gyp info ok
```

(an alternative is to do it manually, but that's more prone to errors. You need to place it in the right folder)

### Test it!
Go to your dev vm, get your node-mapnik dir clean, update, install and give the tests a try:
```
cd node-mapnik
git checkout cdb-1.x
git clean -dxfn # careful!!
npm install
npm test
```

### The rest of it
OK! if you reached this point, the hard part is all done, congrats!

But there's still stuff to do: basically update windshaft and everything that depends on node-mapnik, directly or indirectly:

* tilelive-mapnik
* tilelive-bridge
* mapnik-pool
* abaculus

*NOTE*: this list is not meant to be complete nor up to date by the time you read it. You're better off by running `npm install` and then `npm list` to inspect the dependency tree. And then update everything from the bottom up.

Then update the following:

* windshaft
* windshaft-cartodb

and finally deploy it to production.

#### Upgrade tilelive-mapnik
You can use this [commit](https://github.com/CartoDB/tilelive-mapnik/commit/a8acc03be823383384d990f9983f3a2c6dc6972d) as a reference
```
git clone git@github.com:CartoDB/tilelive-mapnik.git
cd tilelive-mapnik
git checkout cdb-0.x
vim package.json
```

As usual, it is always good to test it:
```
npm install
npm test
```

Then tag and push it:
```
git push origin cdb-0.x
git tag -a '0.6.15-cdb7' -m 'Upgrade node-mapnik to 1.4.15-cdb8'
git push --tags origin 0.6.15-cdb7
```

#### Upgrade the rest of windshaft modules
Basically you can follow the same procedure for mapnik-pool, tilelive-bridge and abaculus.

#### Upgrade windshaft
Follow this example: https://github.com/CartoDB/Windshaft/commit/7d99a35

Then test it, tag it, etc.

#### Upgrade windshaft-cartodb and deploy
TODO
