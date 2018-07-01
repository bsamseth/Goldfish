from time import time

class Conversation():
    def __init__(self, game, engine, xhr, version):
        self.game = game
        self.engine = engine
        self.xhr = xhr
        self.version = version

    command_prefix = "!"

    def react(self, line, game):
        print("*** {} [{}] {}: {}".format(self.game.url(), line.room, line.username, line.text.encode("utf-8")))
        if (line.text[0] == self.command_prefix):
            self.command(line, game, line.text[1:].lower())
        pass

    def command(self, line, game, cmd):
        if cmd == "wait" and game.is_abortable():
            game.abort_in(60)
            self.send_reply(line, "Waiting 60 seconds...")
        elif cmd == "name":
            self.send_reply(line, "{} (lichess-bot v{})".format(self.engine.name(), self.version))
        elif cmd == "howto":
            self.send_reply(line, "How to run your own bot: lichess.org/api#tag/Chess-Bot")
        elif cmd == "eval" and line.room == "spectator":
            stats = self.engine.get_stats()
            self.send_reply(line, ", ".join(stats))
        elif cmd == "eval":
            self.send_reply(line, "I don't tell that to my opponent, sorry.")

    def send_reply(self, line, reply):
        self.xhr.chat(self.game.id, line.room, reply)


class ChatLine():
    def __init__(self, json):
        self.room = json.get("room")
        self.username = json.get("username")
        self.text = json.get("text")
