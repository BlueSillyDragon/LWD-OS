# Nuke built-in rules and variables.
override MAKEFLAGS += -rR

# Convenience macro to reliably declare user overridable variables.
define DEFAULT_VAR =
    ifeq ($(origin $1),default)
        override $(1) := $(2)
    endif
    ifeq ($(origin $1),undefined)
        override $(1) := $(2)
    endif
endef

# Target architecture to build for. Default to x86_64.
override DEFAULT_KARCH := x86_64
$(eval $(call DEFAULT_VAR,KARCH,$(DEFAULT_KARCH)))

override IMAGE_NAME := AquaOS-$(KARCH)

.PHONY: all
all: $(IMAGE_NAME).iso

.PHONY: all-hdd
all-hdd: $(IMAGE_NAME).hdd

.PHONY: run
run: run-$(KARCH)

.PHONY: run-hdd
run-hdd: run-hdd-$(KARCH)

.PHONY: run-x86_64
run-x86_64: ovmf $(IMAGE_NAME).iso
	qemu-system-x86_64 -M q35 -m 2G -bios ovmf-x86_64/OVMF.fd -cdrom $(IMAGE_NAME).iso -boot d

.PHONY: run-hdd-x86_64
run-hdd-x86_64: ovmf $(IMAGE_NAME).hdd
	qemu-system-x86_64 -M q35 -m 2G -bios ovmf-x86_64/OVMF.fd -hda $(IMAGE_NAME).hdd

.PHONY: run-aarch64
run-aarch64: ovmf $(IMAGE_NAME).iso
	qemu-system-aarch64 -M virt -cpu cortex-a72 -device ramfb -device qemu-xhci -device usb-kbd -m 2G -bios ovmf-aarch64/OVMF.fd -cdrom $(IMAGE_NAME).iso -boot d

.PHONY: run-hdd-aarch64
run-hdd-aarch64: ovmf $(IMAGE_NAME).hdd
	qemu-system-aarch64 -M virt -cpu cortex-a72 -device ramfb -device qemu-xhci -device usb-kbd -m 2G -bios ovmf-aarch64/OVMF.fd -hda $(IMAGE_NAME).hdd

.PHONY: run-riscv64
run-riscv64: ovmf $(IMAGE_NAME).iso
	qemu-system-riscv64 -M virt -cpu rv64 -device ramfb -device qemu-xhci -device usb-kbd -m 2G -drive if=pflash,unit=0,format=raw,file=ovmf-riscv64/OVMF.fd -device virtio-scsi-pci,id=scsi -device scsi-cd,drive=cd0 -drive id=cd0,format=raw,file=$(IMAGE_NAME).iso

.PHONY: run-hdd-riscv64
run-hdd-riscv64: ovmf $(IMAGE_NAME).hdd
	qemu-system-riscv64 -M virt -cpu rv64 -device ramfb -device qemu-xhci -device usb-kbd -m 2G -drive if=pflash,unit=0,format=raw,file=ovmf-riscv64/OVMF.fd -device virtio-scsi-pci,id=scsi -device scsi-hd,drive=hd0 -drive id=hd0,format=raw,file=$(IMAGE_NAME).hdd

.PHONY: run-loongarch64
run-loongarch64: ovmf $(IMAGE_NAME).iso
	qemu-system-loongarch64 -M virt -cpu la464 -device ramfb -device qemu-xhci -device usb-kbd -m 2G -bios ovmf-loongarch64/OVMF.fd -cdrom $(IMAGE_NAME).iso -boot d

.PHONY: run-hdd-loongarch64
run-hdd-loongarch64: ovmf $(IMAGE_NAME).hdd
	qemu-system-loongarch64 -M virt -cpu la464 -device ramfb -device qemu-xhci -device usb-kbd -m 2G -bios ovmf-loongarch64/OVMF.fd -hda $(IMAGE_NAME).hdd

.PHONY: run-bios
run-bios: $(IMAGE_NAME).iso
	qemu-system-x86_64 -M q35 -m 2G -cdrom $(IMAGE_NAME).iso -boot d

.PHONY: run-hdd-bios
run-hdd-bios: $(IMAGE_NAME).hdd
	qemu-system-x86_64 -M q35 -m 2G -hda $(IMAGE_NAME).hdd

.PHONY: ovmf
ovmf: ovmf-$(KARCH)

ovmf-x86_64:
	mkdir -p ovmf-x86_64
	cd ovmf-x86_64 && curl -o OVMF.fd https://retrage.github.io/edk2-nightly/bin/RELEASEX64_OVMF.fd

ovmf-aarch64:
	mkdir -p ovmf-aarch64
	cd ovmf-aarch64 && curl -o OVMF.fd https://retrage.github.io/edk2-nightly/bin/RELEASEAARCH64_QEMU_EFI.fd

ovmf-riscv64:
	mkdir -p ovmf-riscv64
	cd ovmf-riscv64 && curl -o OVMF.fd https://retrage.github.io/edk2-nightly/bin/RELEASERISCV64_VIRT_CODE.fd && dd if=/dev/zero of=OVMF.fd bs=1 count=0 seek=33554432

ovmf-loongarch64:
	mkdir -p ovmf-loongarch64
	cd ovmf-loongarch64 && curl -o OVMF.fd https://raw.githubusercontent.com/limine-bootloader/firmware/trunk/loongarch64/QEMU_EFI.fd

limine/limine:
	rm -rf limine
	git clone https://github.com/limine-bootloader/limine.git --branch=v8.x-binary --depth=1
	$(MAKE) -C limine

kernel-deps:
	./kernel/get-deps
	touch kernel-deps

.PHONY: kernel
kernel: kernel-deps
	$(MAKE) -C kernel

$(IMAGE_NAME).iso: limine/limine kernel
	rm -rf iso_root
	mkdir -p iso_root/boot
	cp -v kernel/bin-$(KARCH)/kernel iso_root/boot/
	mkdir -p iso_root/boot/limine
	cp -v limine.conf iso_root/boot/limine/
	mkdir -p iso_root/EFI/BOOT
ifeq ($(KARCH),x86_64)
	cp -v limine/limine-bios.sys limine/limine-bios-cd.bin limine/limine-uefi-cd.bin iso_root/boot/limine/
	cp -v limine/BOOTX64.EFI iso_root/EFI/BOOT/
	cp -v limine/BOOTIA32.EFI iso_root/EFI/BOOT/
	xorriso -as mkisofs -b boot/limine/limine-bios-cd.bin \
		-no-emul-boot -boot-load-size 4 -boot-info-table \
		--efi-boot boot/limine/limine-uefi-cd.bin \
		-efi-boot-part --efi-boot-image --protective-msdos-label \
		iso_root -o $(IMAGE_NAME).iso
	./limine/limine bios-install $(IMAGE_NAME).iso
else ifeq ($(KARCH),aarch64)
	cp -v limine/limine-uefi-cd.bin iso_root/boot/limine/
	cp -v limine/BOOTAA64.EFI iso_root/EFI/BOOT/
	xorriso -as mkisofs \
		--efi-boot boot/limine/limine-uefi-cd.bin \
		-efi-boot-part --efi-boot-image --protective-msdos-label \
		iso_root -o $(IMAGE_NAME).iso
else ifeq ($(KARCH),riscv64)
	cp -v limine/limine-uefi-cd.bin iso_root/boot/limine/
	cp -v limine/BOOTRISCV64.EFI iso_root/EFI/BOOT/
	xorriso -as mkisofs \
		--efi-boot boot/limine/limine-uefi-cd.bin \
		-efi-boot-part --efi-boot-image --protective-msdos-label \
		iso_root -o $(IMAGE_NAME).iso
else ifeq ($(KARCH),loongarch64)
	cp -v limine/limine-uefi-cd.bin iso_root/boot/limine/
	cp -v limine/BOOTLOONGARCH64.EFI iso_root/EFI/BOOT/
	xorriso -as mkisofs \
		--efi-boot boot/limine/limine-uefi-cd.bin \
		-efi-boot-part --efi-boot-image --protective-msdos-label \
		iso_root -o $(IMAGE_NAME).iso
endif
	rm -rf iso_root

$(IMAGE_NAME).hdd: limine/limine kernel
	rm -f $(IMAGE_NAME).hdd
	dd if=/dev/zero bs=1M count=0 seek=64 of=$(IMAGE_NAME).hdd
	sgdisk $(IMAGE_NAME).hdd -n 1:2048 -t 1:ef00
ifeq ($(KARCH),x86_64)
	./limine/limine bios-install $(IMAGE_NAME).hdd
endif
	mformat -i $(IMAGE_NAME).hdd@@1M
	mmd -i $(IMAGE_NAME).hdd@@1M ::/EFI ::/EFI/BOOT ::/boot ::/boot/limine
	mcopy -i $(IMAGE_NAME).hdd@@1M kernel/bin-$(KARCH)/kernel ::/boot
	mcopy -i $(IMAGE_NAME).hdd@@1M limine.conf ::/boot/limine
ifeq ($(KARCH),x86_64)
	mcopy -i $(IMAGE_NAME).hdd@@1M limine/limine-bios.sys ::/boot/limine
	mcopy -i $(IMAGE_NAME).hdd@@1M limine/BOOTX64.EFI ::/EFI/BOOT
	mcopy -i $(IMAGE_NAME).hdd@@1M limine/BOOTIA32.EFI ::/EFI/BOOT
else ifeq ($(KARCH),aarch64)
	mcopy -i $(IMAGE_NAME).hdd@@1M limine/BOOTAA64.EFI ::/EFI/BOOT
else ifeq ($(KARCH),riscv64)
	mcopy -i $(IMAGE_NAME).hdd@@1M limine/BOOTRISCV64.EFI ::/EFI/BOOT
else ifeq ($(KARCH),loongarch64)
	mcopy -i $(IMAGE_NAME).hdd@@1M limine/BOOTLOONGARCH64.EFI ::/EFI/BOOT
endif

.PHONY: clean
clean:
	if test -f kernel-deps; then $(MAKE) -C kernel clean; fi
	rm -rf iso_root $(IMAGE_NAME).iso $(IMAGE_NAME).hdd

.PHONY: distclean
distclean:
	if test -f kernel-deps; then $(MAKE) -C kernel distclean; fi
	rm -rf iso_root *.iso *.hdd kernel-deps limine ovmf*
