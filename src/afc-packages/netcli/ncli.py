#!/usr/bin/env python3
#
# Copyright (C) 2021 Broadcom. All rights reserved. The term "Broadcom"
# refers solely to the Broadcom Inc. corporate affiliate that owns
# the software below. This work is licensed under the OpenAFC Project License,
# a copy of which is included with this software program
#

"""
Description

test
"""

import os
import inspect
import logging
import sys
import uuid
from kombu.mixins import ConsumerMixin
from kombu import Queue, Exchange, Connection, Producer

app_log = logging.getLogger(__name__)


class MsgAcceptor(ConsumerMixin):
    """ Accept messages from broadcast queue and handle them. """
    message_handler = None
    handler_params = None

    def __init__(self, broker_url, broker_exch,
                 msg_handler=None, handler_params=None) -> None:
        app_log.debug(f"({os.getpid()}) {self.__class__.__name__}()")
        self.connection = Connection(broker_url)
        if msg_handler is not None:
            self.message_handler = msg_handler
            self.handler_params = handler_params
        self.exchange = Exchange(
            broker_exch,
            auto_delete=True,
            type='fanout', durable=True)
        self.queue = Queue('dispatcher_' + str(uuid.uuid4().int)[:7],
                           self.exchange, exclusive=True)
        self.queue.no_ack = True

    def get_consumers(self, Consumer, channel):
        return [Consumer(queues=self.queue, callbacks=[self.accept_message])]

    def accept_message(self, body, message):
        if self.message_handler:
            self.message_handler(self.handler_params, body)


class MsgPublisher():
    """ Connection maker """

    def __init__(self, broker_url, broker_exch) -> None:
        app_log.debug(f"({os.getpid()}) {self.__class__.__name__}()")
        self.connection = Connection(broker_url)
        self.exchange = Exchange(
            broker_exch,
            auto_delete=True,
            type='fanout', durable=True)
        self.channel = self.connection.channel()
        self.producer = Producer(exchange=self.exchange,
                                 channel=self.channel)

    def publish(self, msg):
        self.producer.publish(msg)

    def close(self):
        self.connection.release()


# Local Variables:
# mode: Python
# indent-tabs-mode: nil
# python-indent: 4
# End:
#
# vim: sw=4:et:tw=80:cc=+1
