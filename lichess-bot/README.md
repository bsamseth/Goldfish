# Setup

## Install

```bash
virtualenv .lichess-bot-venv
source .lichess-bot-venv/bin/activate
pip install -r ../external/lichess-bot/requirements.txt
```

## Run

It needs to run from lichess-bot's own root, so it's a little nasty.
```bash
export LICHESS_BOT_TOKEN=lip_abcdefg  # Replace with actual token.
cd external/lichess-bot
../../lichess-bot/.lichess-bot-venv/bin/python lichess-bot.py -c ../../lichess-bot/lichess-config.yml --disable_auto_logging
```


## Service 

Put this in `~/.config/systemd/user/goldfish.service`. The example here assumes the user that should run this is
called `pi`, and that this repo is at `/home/pi/git/Goldfish`.

```config
[Unit]
Description=Goldfish lichess-bot
After=network-online.target
Wants=network-online.target

[Service]
Environment="LICHESS_BOT_TOKEN=replace-this-with-a-token"
ExecStart=/home/pi/git/Goldfish/lichess-bot/.lichess-bot-venv/bin/python /home/pi/git/Goldfish/external/lichess-bot/lichess-bot.py --config /home/pi/git/Goldfish/lichess-bot/lichess-config.yml --disable_auto_logging
WorkingDirectory=/home/pi/git/Goldfish/external/lichess-bot
Restart=always

[Install]
WantedBy=multi-user.target
```
