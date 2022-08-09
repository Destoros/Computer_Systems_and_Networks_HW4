# coding: utf-8

from __future__ import absolute_import

from flask import json
from six import BytesIO

from swagger_server.models.temperature_item import TemperatureItem  # noqa: E501
from swagger_server.test import BaseTestCase


class TestDevelopersController(BaseTestCase):
    """DevelopersController integration test stubs"""

    def test_read_temperature(self):
        """Test case for read_temperature

        returns temperature reading of sensor on raspi
        """
        response = self.client.open(
            '/csn_ddi/Temp_API/1.0.0/temperature',
            method='GET')
        self.assert200(response,
                       'Response body is : ' + response.data.decode('utf-8'))


if __name__ == '__main__':
    import unittest
    unittest.main()
