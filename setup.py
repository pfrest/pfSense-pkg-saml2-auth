# Copyright 2025 Jared Hendrickson
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#    http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
"""A setup.py to package our tests written in Python, namely e2e tests."""
from setuptools import setup


def read_readme():
    """Opens and reads the main README.md file for this repo."""
    with open("README.md", "r", encoding="utf-8") as readme_file:
        return readme_file.read()


def read_requirements():
    """Opens and read the requirements.txt file for this repo."""
    with open("requirements-dev.txt", "r", encoding="utf-8") as requirements_file:
        return list(filter(None, requirements_file.read().split("\n")))


# Set our setup parameters
setup(
    name="tests",
    author="Jared Hendrickson",
    author_email="github@jaredhendrickson.com",
    url="https://github.com/pfrest/pfSense-pkg-saml2-auth",
    license="Apache-2.0",
    description="E2E & integration tests for the pfSense-pkg-saml2-auth package written in Python.",
    long_description=read_readme(),
    long_description_content_type="text/markdown",
    version="0.0.0",
    packages=["tests", "tests.helpers", "tests.e2e"],
    install_requires=read_requirements(),
    classifiers=[
        "Programming Language :: Python :: 3",
        "License :: OSI Approved :: Apache Software License",
        "Operating System :: OS Independent",
    ],
    python_requires=">=3.9",
)