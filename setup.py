from os.path import join

from setuptools import Extension, find_packages, setup


def get_long_description():
    with open('README.rst') as fp:
        return fp.read()


def get_ext_modules():
    base = 'ufpeg/booster'
    sources = ['ufpegbooster.cpp']
    depends = ['node.hpp', 'context.hpp', 'instructions.hpp', 'vm.hpp']
    booster = Extension(
        'ufpeg.booster',
        sources=[join(base, src) for src in sources],
        depends=[join(base, dep) for dep in depends],
    )
    return [booster]


setup(
    name='ufpeg',
    version='0.1.0',
    description='UFPEG: An ultra fast parser for Python using PEG',
    long_description=get_long_description(),
    url='https://github.com/eb08a167/ufpeg',
    author='Andrew Kiyko',
    author_email='eb08a167@gmail.com',
    license='MIT',
    classifiers=[
        'Programming Language :: C',
        'Programming Language :: Python :: 3',
        'Intended Audience :: Developers',
    ],
    packages=find_packages(),
    ext_modules=get_ext_modules(),
    python_requires='>=3.3',
)
