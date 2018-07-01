import requests
from urllib.parse import urljoin
from requests.exceptions import ConnectionError, HTTPError
from urllib3.exceptions import ProtocolError

try:
    from http.client import RemoteDisconnected
    # New in version 3.5: Previously, BadStatusLine('') was raised.
except ImportError:
    from http.client import BadStatusLine as RemoteDisconnected

import backoff

ENDPOINTS = {
    "profile": "/api/account",
    "stream": "/api/bot/game/stream/{}",
    "stream_event": "/api/stream/event",
    "game": "/api/bot/game/{}",
    "move": "/api/bot/game/{}/move/{}",
    "chat": "/api/bot/game/{}/chat",
    "abort": "/api/bot/game/{}/abort",
    "accept": "/api/challenge/{}/accept",
    "decline": "/api/challenge/{}/decline",
    "upgrade": "/api/bot/account/upgrade",
    "resign": "/api/bot/game/{}/resign"
}

# docs: https://lichess.org/api
class Lichess():

    def __init__(self, token, url, version):
        self.version = version
        self.header = {
            "Authorization": "Bearer {}".format(token)
        }
        self.baseUrl = url
        self.session = requests.Session()
        self.session.headers.update(self.header)
        self.set_user_agent("?")

    def is_final(exception):
        return isinstance(exception, HTTPError) and exception.response.status_code < 500

    @backoff.on_exception(backoff.expo,
        (RemoteDisconnected, ConnectionError, ProtocolError, HTTPError),
        max_time=120,
        giveup=is_final)
    def api_get(self, path):
        url = urljoin(self.baseUrl, path)
        response = self.session.get(url)
        response.raise_for_status()
        return response.json()

    @backoff.on_exception(backoff.expo,
        (RemoteDisconnected, ConnectionError, ProtocolError, HTTPError),
        max_time=20,
        giveup=is_final)
    def api_post(self, path, data=None):
        url = urljoin(self.baseUrl, path)
        response = self.session.post(url, data=data)
        response.raise_for_status()
        return response.json()

    def get_game(self, game_id):
        return self.api_get(ENDPOINTS["game"].format(game_id))

    def upgrade_to_bot_account(self):
        return self.api_post(ENDPOINTS["upgrade"])

    def make_move(self, game_id, move):
        return self.api_post(ENDPOINTS["move"].format(game_id, move))

    def chat(self, game_id, room, text):
        payload = {'room': room, 'text': text}
        return self.api_post(ENDPOINTS["chat"].format(game_id), data=payload)

    def abort(self, game_id):
        return self.api_post(ENDPOINTS["abort"].format(game_id))

    def get_event_stream(self):
        url = urljoin(self.baseUrl, ENDPOINTS["stream_event"])
        return requests.get(url, headers=self.header, stream=True)

    def get_game_stream(self, game_id):
        url = urljoin(self.baseUrl, ENDPOINTS["stream"].format(game_id))
        return requests.get(url, headers=self.header, stream=True)

    def accept_challenge(self, challenge_id):
        return self.api_post(ENDPOINTS["accept"].format(challenge_id))

    def decline_challenge(self, challenge_id):
        return self.api_post(ENDPOINTS["decline"].format(challenge_id))

    def get_profile(self):
        profile = self.api_get(ENDPOINTS["profile"])
        self.set_user_agent(profile["username"])
        return profile

    def resign(self, game_id):
        self.api_post(ENDPOINTS["resign"].format(game_id))

    def set_user_agent(self, username):
        self.header.update({"User-Agent": "lichess-bot/{} user:{}".format(self.version, username)})
        self.session.headers.update(self.header)
