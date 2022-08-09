# coding: utf-8

from __future__ import absolute_import

from flask import json
from six import BytesIO

from swagger_server.test import BaseTestCase


class TestTemperatureController(BaseTestCase):
    """TemperatureController integration test stubs"""

    def test_get_temperature(self):
        """Test case for get_temperature

        Reads temperature from local sensor and returns value
        """
        response = self.client.open(
            '/v2/getTemperature',
            method='GET')
        self.assert200(response,
                       'Response body is : ' + response.data.decode('utf-8'))


if __name__ == '__main__':
    import unittest
    unittest.main()
