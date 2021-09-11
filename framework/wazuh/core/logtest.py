# Copyright (C) 2021, INO Inc.
# Created by Wazuh, Inc. <info@wazuh.com>.
# This program is a free software; you can redistribute it and/or modify it under the terms of GPLv2

from wazuh.core.common import LOGTEST_SOCKET
from wazuh.core.wazuh_socket import WazuhSocketJSON, create_wazuh_socket_message


def send_logtest_msg(command: str = None, parameters: dict = None):
    """Connect and send a message to the logtest socket.

    Parameters
    ----------
    command: str
        Command to send to the logtest socket.
    parameters : dict
        Dict of parameters that will be sent to the logtest socket.

    Returns
    -------
    dict
        Response from the logtest socket.
    """
    full_message = create_wazuh_socket_message(origin={'name': 'Logtest', 'module': 'api/framework'},
                                               command=command,
                                               parameters=parameters)
    logtest_socket = WazuhSocketJSON(LOGTEST_SOCKET)
    logtest_socket.send(full_message)
    response = logtest_socket.receive(raw=True)
    logtest_socket.close()

    return response
