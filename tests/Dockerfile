FROM python:3.13-slim

COPY ./tests /app/
COPY ./requirements-dev.txt /app/
COPY ./setup.py /app/
COPY ./README.md /app/

# Install necessary dependencies
RUN python3 -m pip install /app/
RUN playwright install && playwright install-deps

# Clean up apt cache to reduce image size
RUN apt-get clean && rm -rf /var/lib/apt/lists/*

# Keep the container running to allow interactive testing
WORKDIR /app/tests
CMD ["tail", "-f", "/dev/null"]