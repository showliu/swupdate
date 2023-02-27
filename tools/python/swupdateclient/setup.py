from setuptools import setup, find_packages

setup(
    name='swupdateclient',
    version='0.1',
    packages=find_packages(),
    url='https://github.com/sbabic/swupdate/tree/master/tools/python/swupdateclient',
    license='GPLv3',
    author='Stefano Babic',
    author_email='stefano.babic@swupdate.org',
    description='Python Client to update SWUpdate based devices',
    entry_points={
        "console_scripts": [
            "swupdateclient=swupdateclient.main:main",
        ],
    },
    python_requires=">=3.9",
)
