<p style="text-align: center; padding-top: 10px">
<span style="vertical-align: top; font-size: 36pt">
<img src="data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAAJAAAACoAQAAAAAnFFrVAAAC3UlEQVR4Ac3WT2sTURAA8Hm7wc0hZGtOAdNuWgpeg7mkGNqmIsaTJz9AKcRr8eSh0K1CewlU/AB+DnvbGCGXgh/AQxMD7dHoqYK8584b3mT/akACvt5+MNOZebOPAB9bKTpzsoCO+G/JOfGhCmBHyB2kqDZJUfXF9PRLf3+49ZWp5tyMri4Pps0fTGXR71Uve82DB0yOIJq+4XnZJyfXIdmDIZMYnGkK5ulhctpDGr9kqu5TrsMmB9Ym/Wuk5pzcwbdr7/N0+nBel+Pf9O6d7Q9XOX32Pf7r1XLgcmjR2xZyEUJDEn4mcaD+A2d8BEeuHyV3JkF6QTTQ+6mE2hlHafViJEatkGwMwoayyPvgEFlCSQCd3s+gMkgKBC0KnGANjuJkj9sgKRANQhKzV2KUpJkpAugfwOrtOEUXYw4MSejqg604Ydtb1DaCDnS/qxYOh0jJkJyJatEIiZKDproyiM7CRFOVOVQEP7FfhWLQb3RiVLq7XukOY/tVQHqUIGF1H3MRAnOtfyykaKPYfeYjgR6PQFrpdiBGnU0kKkIiQnn301pjEiPYLXDboXHbmUQ7gfpnsoRJX7KqpUb7IEqVO5uV7gYSb6G1t/kuRYXX3Y3nplQJmKtYaLVjZFkrxWL9Ke+XpvpKF2k+icpe/W2CSk/a7xv1DhLtOFefTbzkecRto+USrwmnzyUpEDNpSU+tHqlk8lSQSTpMGtpR40zCvg1tU2CUjjXp2Zi2le9dJWjqe+dIiMJQ2faT5Dq+RW8oCEPnPlWvJNP9NCmdC0PVIgTibxRL7yhIE7/3RCpNx6ADo/u1nSYgojhDALkEoeUT7UQuLWvlFiELEq9vPU2H0SeNSVL5Zr+y6BYDNYo8ciAkqn5O4leCyqDUrY3DQsynYzWjezRLXsNPIUm4TNQOk6Mgj5Rpu5VF7hXlwmPoPE34DVEVhtbsIEk1EVBDhgCqkCLeCbkAoWTRkn9Q/ga8ZUgsuIBPrAAAAABJRU5ErkJggg==" />*</span>
</p>

PebbleBucks 2.0
===============

PebbleBucks is an app for the [Pebble Smartwatch](https://getpebble.com) that allows easy access to your Starbucks barcode and account information, including your dollar ballance, free drink count, count of drinks until your next free drink, and last updated date. The select button updates the account information from Starbucks.

Based on [PebbleBucks 2.0 by Neal](https://github.com/Neal/PebbleBucks) and the [original PebbleBucks by matt Donders](https://github.com/mattdonders/PebbleBucks).
Idea and images are from the original PebbleBucks by mattdonders.

___*___ The above barcode was generated with a random 16-digit number. If it turns out that this random 16-digit code is a valid Starbucks card number, whoops and sorry.

## Getting Started

* Download and install the watch app from the [Releases](https://github.com/a2/PebbleBucks/releases) page.
* Configure the watchapp with your card number and Starbucks login credentials.

## Wait, what are you going to do with my data?

### Card Number

The card number is used to generate a barcode for use at Starbucks by a script that runs locally on your phone. This barcode is transmitted to your Pebble watch and is not saved anywhere else.

### Starbucks Credentials

Your Starbucks login credentials are stored on your phone. They are only transferred to the Starbucks server using the secure HTTPS protocol.
