# This Python file uses the following encoding: utf-8

from setuptools import setup, Extension

setup(
    name = "toymath",
    ext_modules=[
        Extension("toymath", ["toymath.cpp"]),
    ]
)

