A mod by human for human, I do not like the usage of generative AIs. Also, we stand for Trans people right :3

# TwitchLoader Mod

A [Dusklight](https://github.com/TwilitRealm/dusklight) mod.
Provides a service to link dusklight with twitch events (with oauth token and with websockets) to be used with others for gameplay 

## Quick start

See [mod-template](https://github.com/TwilitRealm/mod-template) for further documentation

## Building

As of right now, I work with linux with distrobox, ubuntu v24, since most tutorial will be ubuntu compliant. Sorry fellow people on mac and windows, I might update this later.
(It could be as straightforward as following dusklight main documentation for building and then adapting the few more needed here)

   ```sh
   distrobox create -i ubuntu:24.04 -n ubuntu-box
   distrobox enter ubuntu-box
   
   # Follow dusklight main repo documentation
   
   # then for building this mod specifically, it needs libraries to communicate with websocket in ssl
   sudo apt install libboost-dev libboost-system-dev libssl-dev

   # and then simply build it :)
   cmake -B build
   cmake --build build
   ```

## Disclaimer

I am using [Boost's beast library](https://github.com/boostorg/boost) for the websockets and I have absolutely no clue how license works, please do tell me if I'm doing anything forbidden. Also, feel free to use this mod as a base for twitch integration.
I like sequence diagrams, they help to properly visualize behaviors of what's happening in the code.

## TODOs

- First line
- clear the code of all the example and be as concise as possible (the template is still there)
- create sequence diagrams for what I expect of this mod to become
- define a convention for versioning of the mod (most probably x.y.z : x major and breaks the service contract, y minor no modification of naming functions nor main behavior, z hotfixes typo etc)

## My other mods to be used with TwitchLoader

The first one will simply be notifications and a proof of concept for what can be achieved.
However, they will both live for the few firsts commits

## Anything else 

I am open to suggestions, questions, feel free to ask with an issue.
It is possible that I forgot stuffs, it will come at a later date :>